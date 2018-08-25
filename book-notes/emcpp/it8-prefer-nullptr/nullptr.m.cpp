#include <iostream>
#include <string>
#include <mutex>

using namespace std;

int    f1(std::shared_ptr<int> spw) { return 0; };  // call these only when
double f2(std::unique_ptr<int> upw) { return 0.0; };  // the appropriate
bool   f3(int* pw) { return true; };                   // mutex is locked

template<typename FuncType,
         typename MuxType,
         typename PtrType>
auto lockAndCall(FuncType func,
                 MuxType& mutex,
                 PtrType ptr) -> decltype(func(ptr)) // C++11, in 14, do decltype(auto)
{
  using MuxGuard = std::lock_guard<MuxType>;

  MuxGuard g(mutex);
  return func(ptr);
}


int main() {
  std::mutex f1m, f2m, f3m;

  //auto result1 = lockAndCall(f1, f1m, 0);          // error: no known conversion from int to shared_ptr!
  //auto result2 = lockAndCall(f2, f2m, NULL);       // error: no known conversion from long to unique_ptr!
  auto result3 = lockAndCall(f3, f3m, nullptr);    // fine
  auto result4 = lockAndCall(f2, f3m, nullptr);    // fine
  auto result5 = lockAndCall(f1, f3m, nullptr);    // fine
  return 0;
}