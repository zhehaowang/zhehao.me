#include <iostream>
#include <string>
#include <thread>
#include <future>

using namespace std::literals;

void doAsyncWork() noexcept {
  std::this_thread::sleep_for(1s);
}

int main() {
// A quick look at std::thread and std::async (future)
  //auto fut = std::async(doAsyncWork); // default launch policy, can choose deferred
  //auto fut = std::async(std::launch::deferred, doAsyncWork);  // deferred, the loop never stops
  auto fut = std::async(std::launch::async, doAsyncWork);     // async, works just fine

  while (fut.wait_for(100ms) != std::future_status::ready) {
    std::cout << "waiting\n";
  }
  std::cout << "done\n";
  return 0;

  // to fix this, we could add
  /*
  if (fut.wait_for(0s) == std::future_status::deferred) {
    fut.get();
    ...
  } else {
    while (fut.wait_for(100ms) != std::future_status::ready) {
      std::cout << "waiting\n";
    }
    std::cout << "done\n";
  }
  */
}