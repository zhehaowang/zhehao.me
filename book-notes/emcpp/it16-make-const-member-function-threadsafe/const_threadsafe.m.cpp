#include <iostream>
#include <mutex>
#include <vector>
#include <string>

// demonstrates a case where a const member function is only made threadsafe by
// introducing a mutex. The particular example does not involve a multi-threaded
// environment.

// Does const member function mean it's threadsafe?
// No, they could be operating on 'mutable' states.
//
// Should you make them threadsafe?
// Yes. Unless you are sure this code will run in a single-threaded environment.
// (alternatively, you could document the threadsafety behavior like BDE
// components are required to do)
//
// The mutex in this example guarantees threadsafety.

class Polynomial {
public:
  using RootsType = std::vector<double>;

  RootsType roots() const {
    std::lock_guard<std::mutex> g(m);     // lock mutex

    if (!rootsAreValid) {            // if cache not valid
      computeRoots();
                                     // compute roots,
                                     // store them in rootVals
      rootsAreValid = true;
    }

    return rootVals;
  }

private:
  void computeRoots() const {}

  mutable std::mutex m;
  mutable bool rootsAreValid{ false };    // see Item 7 for info
  mutable RootsType rootVals{};           // on initializers
};

int main() {
  Polynomial p;
  return 0;
}