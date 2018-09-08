#include <iostream>
#include <string>
#include <thread>
#include <future>

using namespace std::literals;

int main() {
  std::atomic<int> ai(0);
  volatile int vi = 0;

  std::thread t1([&ai, &vi](){
    ai++;
    vi++;
  });

  std::thread t2([&ai, &vi](){
    ai++;
    vi++;
  });

  // volatile has nothing to do with concurrency
  t1.join();
  t2.join();
  std::cout << "atomic: " << ai.load() << "\nvolatile: " << vi << "\n";

  // volatile does prevent optimization of the following states
  volatile int vi1 = vi;
  vi1 = vi;   // will not be optimized out
  vi1 = 10;   // will not be optimized out
  vi1 = 20;
  // why does this matter? Not in this process but if another process shares
  // this memory, or has memory mapped IO, then this kind of optimization
  // leads to wrong behaviors.
  return 0;
}