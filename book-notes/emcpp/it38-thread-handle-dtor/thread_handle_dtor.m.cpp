#include <iostream>
#include <string>
#include <thread>
#include <future>

using namespace std::literals;

bool doWork() {
  std::cout << "job launch\n";
  std::this_thread::sleep_for(1s);
  std::cout << "job done\n";
  return true;
}

int main() {
// A quick look at different behaviors in std::future dtor
  std::cout << "fut1 (async):\n";
  {
    auto fut = std::async(std::launch::async, doWork);
    // implicit join on fut dtor
  }
  std::cout << "fut2 (packaged_task):\n";
  std::packaged_task<bool()> pt(doWork);    // can run asynchronously
  
  {
    auto fut1 = pt.get_future();               // get future for pt
    std::thread t(std::move(pt));
    // just destroy members on fut1 dtor
    
    // do nothing, program halts on t dtor

    // do detach
    t.detach();

    // do join
    //t.join();

    // in any case, dtor of fut1 just destroys data members,
    // no implicit join or detach
  }
  std::cout << "program done";
  return 0;
}