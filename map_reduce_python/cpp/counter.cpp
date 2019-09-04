#include "tbb/concurrent_queue.h"
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <regex>
#include <unordered_map>

using namespace std;

const string M_DATA = "M_DATA";
const string M_END = "M_END";

template <class T> using Message = pair<string, T>;

template <class T>
using MessageQueue = tbb::concurrent_bounded_queue<Message<T>>;

using Counter = unordered_map<string, int>;

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

void count_words(MessageQueue<string> in_queue,
                 MessageQueue<Counter> to_reducer,
                 MessageQueue<void*> to_reader) {

  Message<string> message;
  while (true) {
    in_queue.pop(message);
    if (message.first == M_END) {
      break;
    }
    Counter counter = _count_words(message.second);

    to_reducer.push(Message<Counter>(M_DATA, move(counter)));
    to_reader.push(Message<void*>(M_DATA, nullptr));
  }
}

int main() {
  MessageQueue<string> queue;
  thread t1(process_one<string>, ref(queue));
  thread t2(process_two<string>, ref(queue));
  t1.join();
  t2.join();
}