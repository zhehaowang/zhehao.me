#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <mutex>

using namespace std::literals;

void detect() {
  std::cout << "detect start\n";
  std::this_thread::sleep_for(1s);
  std::cout << "detect stop\n";
  return;
}

void react() {
  std::cout << "react fired\n";
  return;
}

// Condition variable to achieve synchronization
void condVar() {
  std::condition_variable cv;
  std::mutex m;
  
  std::thread t([&m, &cv](){
    std::unique_lock<std::mutex> lk(m);
    // if wait is called after notify_one (not possible in this case)
    // this will hang forever
    cv.wait(lk);
    react();
  }
  // not quite right: this misses the check for spurious wakeups
  );

  detect();
  cv.notify_one();

  // better yet, use ThreadRAII
  t.join();
}

// Busy wait flag approach to achieve synchronization
void busyWaitFlag() {
  std::atomic<bool> flag(false);

  std::thread t([&flag](){
    while (!flag) {
      std::this_thread::sleep_for(100ms);
    }
    react();
  });

  detect();
  flag = true;

  // better yet, use ThreadRAII
  t.join();
}

// condition variable + flag to achieve synchronization
void condVarPlusFlag() {
  std::condition_variable cv;
  std::mutex m;
  // note that the bool does not need to be atomic
  bool flag(false);
  
  std::thread t([&m, &cv, &flag](){
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [&flag](){
      return flag;
    } // spurious wakeup detection
    );
    react();
  });

  detect();
  {
    std::lock_guard<std::mutex> guard(m);
    flag = true;
  }
  cv.notify_one();

  // better yet, use ThreadRAII
  t.join();
}

void future() {
  std::promise<void> p;
  std::thread t([&p](){
    p.get_future().wait();
    react();
  });
  detect();
  // if detect throws, the program will crash as t is joinable when
  // this stack unwinds.
  // if we switch to using ThreadRAII with DtorAction::join, the thread
  // will hang on wait call forever.
  p.set_value();
  // this signaling is one-off, involves heap allocation, but does not
  // have the code smell from earlier.

  t.join();
  // a common use case for this could be to instantiate the thread in
  // a blocked state, using its native_handle to configure priority,
  // affinity, etc, then execute a set_value and start the thread.
}

int main() {
  //condVar();
  //busyWaitFlag();
  //condVarPlusFlag();
  future();
  return 0;
}