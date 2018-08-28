#include <iostream>
#include <string>
#include <vector>
#include <mutex>

// Recommended practice, if the default supplied behaviors are desired
// Understand clearly state your intentt

class Base {
public:
  virtual ~Base() = default;                // make dtor virtual

  Base(Base&&) = default;                   // support moving
  Base& operator=(Base&&) = default;

  Base(const Base&) = default;              // support copying
  Base& operator=(const Base&) = default;
};

int main() {
  return 0;
}