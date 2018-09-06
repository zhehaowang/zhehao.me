#include <iostream>
#include <string>
#include <thread>
#include <future>

int doAsyncWork(int a, int b) noexcept {
  std::cout << "doAsyncWork(" << a << ", " << b << ")\n";
  return a * b;
}

int main() {
// A quick look at std::thread and std::async (future)
  std::thread t(doAsyncWork, 4, 5);

  auto future = std::async(doAsyncWork, 3, 4);
  auto res = future.get();
  std::cout << res << "\n";

  // crashes without joining t to main
  t.join();
  return 0;
}