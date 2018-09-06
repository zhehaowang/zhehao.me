#include <iostream>
#include <string>
#include <thread>
#include <vector>

class ThreadRAII {
public:
  enum class DtorAction { join, detach };    // see Item 10 for
                                             // enum class info

  ThreadRAII(std::thread&& t, DtorAction a)  // in dtor, take
  : action(a), t(std::move(t)) {}            // action a on t
  // in ctor, note that we only move thread to be managed by this
  // RAII object, note that we can't copy std::thread objects
  
  ~ThreadRAII()
  {
    if (t.joinable()) {
      // joinability test is necessary since join or detach on
      // unjoinable threads yields UB.
      if (action == DtorAction::join) {
        t.join();
        // If you are worried about the potential race condition
        // between joinable check and join / detach actions, such
        // worries are unfounded: a std::thread object can change
        // state from joinable to unjoinable only through a member
        // function call or a move operation. At the time a ThreadRAII
        // object's dtor'ed invoked, no other thread should be making
        // member function calls on that object.
        // The client code still could invoke dtor and something else
        // on the object at the same time, but should be made aware
        // such calls could result in race conditions.
      } else {
        t.detach();
      }
      
    }
  }

  std::thread& get() { return t; }           // see below

  ThreadRAII(ThreadRAII&&) = default;               // support
  ThreadRAII& operator=(ThreadRAII&&) = default;    // moving
private:
  DtorAction action;
  std::thread t;
  // note the order of data members. In this case it doesn't matter
  // but you usually want to put them to last in a class's members
  // since once initialized they may start running immediately, and
  // running them may require other member variables to be already
  // initialized.
};

bool conditionsAreSatisfied() {
  return false;
}

bool doWork(std::function<bool(int)> filter,  // as before
            int maxVal = 10'000'000)
{
  std::vector<int> goodVals;                  // as before

  ThreadRAII t(                               // use RAII object
    std::thread([&filter, maxVal, &goodVals]
                {                             
                  for (auto i = 0; i <= maxVal; ++i)
                    { if (filter(i)) goodVals.push_back(i); }
                }),
                ThreadRAII::DtorAction::join  // RAII action
  );

  /*
  std::thread t([&filter, maxVal, &goodVals]
                {                             
                  for (auto i = 0; i <= maxVal; ++i)
                    { if (filter(i)) goodVals.push_back(i); }
                });
  */

  //auto nh = t.get().native_handle();

  if (conditionsAreSatisfied()) {
    t.get().join();
    //t.join();
    std::cout << "perform computation\n";
    return true;
  }

  return false;
}

int main() {
// A quick look at std::thread and std::async (future)
  doWork([](int x) {
    return x % 10 == 0;
  });
}