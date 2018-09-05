#include <iostream>
#include <string>
#include <chrono>

using Time = std::chrono::steady_clock::time_point;

using namespace std::chrono;
using namespace std::literals;
using namespace std::placeholders;

using Duration = std::chrono::steady_clock::duration;

enum class Sound { Beep, Siren, Whistle };

// at time t, make sound s for duration d
void setAlarm(Time t, Sound s, Duration d) {
  std::cout << "make sound " << static_cast<int>(s) << "\n";
}

int main() {
// Comparison of currying using lambdas and binds
  auto duration = 30s;
  auto setSoundB =
    std::bind(setAlarm,
              // note how a 2nd bind is needed here to get the desired
              // behavior of evaluting now at setSoundB call instead
              // of at bind call
              std::bind(std::plus<>(),
                        std::bind(steady_clock::now),
                        1h),
              // note how the placeholder _1 does not convey afterwards
              // when calling setSoundB is this argument passed by value
              // or reference into setAlarm. (Reference)
              _1,
              // note also it's not clear how the captured values are
              // stored inside this bind object: by reference or value.
              // (Value)
              duration);
  
  auto setSoundL =                             
    [duration](Sound s)
    {
      setAlarm(steady_clock::now() + 1h,  // alarm to go off
               s,                         // in an hour for
               duration);                 // 30 seconds
  };

  setSoundB(Sound::Beep);
  setSoundL(Sound::Siren);
  return 0;
}