import re
import sys
from collections import defaultdict, Counter
from multiprocessing import Queue, Process, Pipe, cpu_count

from utils import timeit

CHUNK_SIZE = 4096 * 2 ** 13  # ~33 MB

M_DATA = 'M_DATA'
M_END = 'M_END'


# message, payload = input

def _count_words(data):
    # counter = defaultdict(int)
    # for i in re.finditer(rb'\w+', data):
    #     counter[i.group()] += 1
    # for i in data.split():
        # counter[i] += 1
    
    counter = Counter(data.split())

    return counter


def _read_file(name, chunk_size=CHUNK_SIZE):
    with open(name, 'rb') as bin_file:
        while True:
            data = bin_file.read(chunk_size)
            if not data:
                break
            yield data


def count_words(in_queue, to_reducer, to_reader):
    """
    Count words in a string.
    :param in_queue: Input queue
    :param to_reducer: Output queue, send messages to reducer.
    :param to_reader: Output queue, send messages to reducer.
    """
    while True:
        message, payload = in_queue.get()
        if message == M_END:
            break

        counts = _count_words(payload)

        to_reducer.put((M_DATA, counts))
        to_reader.put((M_DATA, None))


def reduce(in_queue, pipe_out):
    """
    Reduce the word counts in one dict.
    :param in_queue: Input queue.
    :param pipe_out: Output pipe, send messages to the parent process.
    """
    # counts = defaultdict(int)
    counts = Counter()
    while True:
        message, payload = in_queue.get()
        if message == M_END:
            break
    counts.update(payload)
        # for word, times in payload.items():
            # counts[word] += times

    pipe_out.send(counts)


def read_file(file_name, nworkers, in_queue, out_queue):
    """
    Read data from a big file
    :param file_name: Name of the input file.
    :param nworkers: Number of workers (word counters).
    :param in_queue: Input queue.
    :param out_queue: Output queue, send messages to word counter.
    """
    file_iterator = _read_file(file_name)

    # First read two full chunks of data
    for i in range(2):
        data = next(file_iterator, None)
        if data is None:
            return

        size = len(data) // nworkers
        for j in range(nworkers):
            out_queue.put((M_DATA, data[j * size: (j + 1) * size]))

    # Then load data and wait...
    for data in _read_file(file_name):
        # Wait for N workers to complete its job before sending more data
        for j in range(nworkers):
            in_queue.get()

        size = len(data) // nworkers
        for j in range(nworkers):
            out_queue.put((M_DATA, data[j * size: (j + 1) * size]))

    for i in range(nworkers):
        out_queue.put((M_END, None))


@timeit
def map_reduce(name):
    nworkers = cpu_count()

    to_counter = Queue()
    to_reader = Queue()
    to_reducer = Queue()
    receiver, sender = Pipe(False)

    reducer = Process(target=reduce, args=(to_reducer, sender), daemon=True)
    reader = Process(target=read_file,
                     args=(name, nworkers, to_reader, to_counter),
                     daemon=True)

    workers = []
    for i in range(nworkers):
        worker = Process(target=count_words,
                         args=(to_counter, to_reducer, to_reader),
                         daemon=True)
        workers.append(worker)

    reader.start()
    reducer.start()
    for w in workers:
        w.start()

    reader.join()

    for w in workers:
        w.join()

    to_reducer.put((M_END, None))
    data = receiver.recv()
    reducer.join()
    return data


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Please include the file name as argument.')
        exit(-1)

    map_reduce(sys.argv[1])
