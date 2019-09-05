#include "mtime.hpp"
#include "tbb/concurrent_queue.h"
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <deque>

using namespace std;

const size_t CHUNK_SIZE = 4096 * (1 << 13);

const string M_DATA = "M_DATA";
const string M_END = "M_END";

template <class T> using Message = pair<string, T>;

template <class T>
using MessageQueue = tbb::concurrent_bounded_queue<Message<T>>;

using Counter = unordered_map<string, int>;

struct ChunkData {
  string data;
  size_t size;
};

template <typename T> void process_one(MessageQueue<T> &input_queue) {
  while (true) {
    Message<T> message;
    input_queue.pop(message);
    cout << message.second << endl;
  }
}

template <typename T> void process_two(MessageQueue<T> &output_queue) {
  while (true) {
    output_queue.push({"A", "mensaje"});
    this_thread::sleep_for(3s);
  }
}

void read_file(const string &name, const int workers,
               MessageQueue<void *> &in_queue,
               MessageQueue<string> &to_counter) {
  string buffer(CHUNK_SIZE, '\0');
  ifstream file(name);

  for (size_t i = 0; i < 2; i++) {
    if (!file) {
      break;
    }
    file.read(&buffer[0], CHUNK_SIZE);
    const size_t chars_readed = file.gcount();

    const size_t size = (chars_readed + workers) / workers;
    for (size_t j = 0; j < workers; j++) {
      size_t start = j * size;
      size_t end = min((j + 1) * size, chars_readed);
      string data = buffer.substr(start, end);
      to_counter.push({M_DATA, move(data)});
    }
  }

  while (file) {
    file.read(&buffer[0], CHUNK_SIZE);
    const size_t chars_readed = file.gcount();

    for (size_t j = 0; j < workers; j++) {
      Message<void *> message;
      in_queue.pop(message);
    }

    const size_t size = (chars_readed + workers) / workers;
    for (size_t j = 0; j < workers; j++) {
      size_t start = j * size;
      size_t end = min((j + 1) * size, chars_readed);
      string data = buffer.substr(start, end);
      to_counter.push({M_DATA, move(data)});
    }
  }
  for (size_t i = 0; i < workers; i++) {
    to_counter.push({M_END, ""});
  }
  file.close();
}

void _count_words(const string &s, Counter & output) {
  deque<string> strings;
  boost::split(strings, s, boost::is_any_of("\n "));

  for (const auto &str : strings) {
    output[str]++;
  }
}

void count_words(MessageQueue<string> &in_queue,
                 MessageQueue<Counter> &to_reducer,
                 MessageQueue<void *> &to_reader) {
  Counter counter;
  Message<string> message;

  in_queue.pop(message);

  while (message.first != M_END) {
    _count_words(message.second, counter);

    to_reader.push({M_DATA, nullptr});
    in_queue.pop(message);
  }
  to_reducer.push({M_DATA, move(counter)});
}

void reduce(MessageQueue<Counter> &in_queue, MessageQueue<Counter> &out_queue) {
  Counter counter;
  Message<Counter> message;

  in_queue.pop(message);
  while (message.first != M_END) {
    for (const auto &data : message.second) {
      counter[data.first] += data.second;
    }

    in_queue.pop(message);
  }

  out_queue.push({M_DATA, counter});
}

Counter map_reduce(const string &file_name) {
  const size_t nworkers = thread::hardware_concurrency();

  MessageQueue<string> to_counter;
  MessageQueue<void *> to_reader;
  MessageQueue<Counter> to_reducer;
  MessageQueue<Counter> output;

  thread reducer(reduce, ref(to_reducer), ref(output));
  thread reader(read_file, ref(file_name), nworkers, ref(to_reader),
                ref(to_counter));

  vector<thread> workers(nworkers);
  for (auto &worker : workers) {
    worker =
        thread(count_words, ref(to_counter), ref(to_reducer), ref(to_reader));
  }

  reader.join();

  for (auto &worker : workers) {
    worker.join();
  }
  to_reducer.push({M_END, {}});

  Message<Counter> data;
  output.pop(data);

  reducer.join();

  return data.second;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Please include the file name as argument." << endl;
    return -1;
  }
  Counter data;

  auto time = mtime::mTime([&]() { data = map_reduce(argv[1]); });

  cout << time / 1000.0 << endl;

  ofstream csv_file("cpp_output.csv");

  for (const auto &pair_ : data) {
    csv_file << pair_.first << " " << pair_.second << '\n';
  }
  csv_file.close();
}
