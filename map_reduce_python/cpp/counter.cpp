#include "tbb/concurrent_queue.h"
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <fstream>
#include <algorithm>

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

Counter _count_words(const string &s) {
  const string rgx_str = "\\s+";
  Counter counter;

  regex rgx(rgx_str);

  sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
  sregex_token_iterator end;

  while (iter != end) {
    counter[*iter]++;
    iter++;
  }

  return counter;
}

void read_file(const string &name, const int workers,
               MessageQueue<void *> &in_queue, MessageQueue<string> &to_counter) {
  string buffer(CHUNK_SIZE, '\0');
  ifstream file(name);

  for(size_t i =0; i < 2; i++) {
    if (!file) {
      break;
    }
    file.read(&buffer[0], CHUNK_SIZE);
    const size_t chars_readed = file.gcount();
    
    const size_t size = (chars_readed + workers ) / workers;
    for (size_t j = 0; j < workers; j++) {
      size_t start = j * size;
      size_t end = min((j + 1) * size, chars_readed);
      string data = buffer.substr(start, end); 
      to_counter.push({M_DATA, data});
    }
  }

  while(file) {
    file.read(&buffer[0], CHUNK_SIZE);
    const size_t chars_readed = file.gcount();

    const size_t size = (chars_readed + workers ) / workers;
    for (size_t j = 0; j < workers; j++) {
      size_t start = j * size;
      size_t end = min((j + 1) * size, chars_readed);
      string data = buffer.substr(start, end); 
      to_counter.push({M_DATA, data});
    }

  } 
  for (size_t i = 0; i < workers; i++) {
    to_counter.push({M_END, ""});
  }


}

void count_words(MessageQueue<string> &in_queue,
                 MessageQueue<Counter> &to_reducer,
                 MessageQueue<void *> &to_reader) {

  Message<string> message;
  while (true) {
    in_queue.pop(message);
    if (message.first == M_END) {
      break;
    }
    Counter counter = _count_words(message.second);

    to_reducer.push(Message<Counter>(M_DATA, move(counter)));
    to_reader.push(Message<void *>(M_DATA, nullptr));
  }
}

void reduce(MessageQueue<Counter> &in_queue, MessageQueue<Counter> &out_queue ) {
  Counter counter;
  while (true) {
    Message<Counter> message;
    in_queue.pop(message);
    if (message.first == M_END) {
      break;
    }
    for (const auto&data : message.second) {
      counter[data.first] += data.second;
    }
  }

  out_queue.push({M_DATA, counter});

}

Counter map_reduce(const string& file_name) {
  const size_t nworkers = thread::hardware_concurrency();

  MessageQueue<string>  to_counter;
  MessageQueue<void *> to_reader;
  MessageQueue<Counter> to_reducer;
  MessageQueue<Counter> output;

  thread reducer(reduce, ref(to_reducer), ref(output));
  thread reader(ref(file_name), nworkers, ref(to_reader), ref(to_counter));

  vector<thread> workers(nworkers);
  for (auto & worker : workers) {
    worker = thread(count_words, ref(to_counter), ref(to_reducer), ref(to_reader));
  }

  reader.join();

  for ( auto & worker : workers) {
    worker.join();
  }
  to_reducer.push({M_END, {}});

  Message<Counter> data;
  output.pop(data);

  reducer.join();

  return data.second;
}

int main() {
  // MessageQueue<string> queue;
  // thread t1(process_one<string>, ref(queue));
  // thread t2(process_two<string>, ref(queue));
  // t1.join();
  // t2.join();

  auto data = map_reduce("../complete.txt");


}