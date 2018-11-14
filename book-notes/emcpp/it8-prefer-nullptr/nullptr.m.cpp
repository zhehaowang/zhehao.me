#include <iostream>
#include <string>
#include <mutex>

using namespace std;

// demonstrates how nullptr is a pointer type, while neither 0 nor NULL is a
// pointer type

int    f1(std::shared_ptr<int> spw) { return 0; };    // call these only when
double f2(std::unique_ptr<int> upw) { return 0.0; };  // the appropriate
bool   f3(int* pw) { return true; };                  // mutex is locked

template<typename FuncType,
         typename MuxType,
         typename PtrType>
auto lockAndCall(FuncType func,
                 MuxType& mutex,
                 PtrType ptr) -> decltype(func(ptr)) // C++11, in 14, do
                                                     // decltype(auto)
{
  using MuxGuard = std::lock_guard<MuxType>;
  MuxGuard g(mutex);
  return func(ptr);
}


int main() {
  std::mutex fm;

  //auto result1 = lockAndCall(f1, fm, 0);        // error: no known conversion
                                                  // from int to shared_ptr!
  //auto result2 = lockAndCall(f2, fm, NULL);     // error: no known conversion
                                                  // from long to unique_ptr!
  auto result3 = lockAndCall(f3, fm, nullptr);    // fine
  auto result4 = lockAndCall(f2, fm, nullptr);    // fine
  auto result5 = lockAndCall(f1, fm, nullptr);    // fine
  return 0;
}