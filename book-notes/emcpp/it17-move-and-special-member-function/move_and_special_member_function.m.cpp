#include <iostream>
#include <string>

class NoMove {
public:
  NoMove() {
    std::cout << "NoMove default ctor\n";
  };
  ~NoMove() = default;
  
  NoMove(NoMove&&) = delete;
  NoMove& operator=(NoMove&&) = delete;

  NoMove(const NoMove& rhs) : y(rhs.y) {
    std::cout << "NoMove copycon\n";
  }

  NoMove& operator=(const NoMove& rhs) {
    std::cout << "NoMove copy assignment\n";
    y = rhs.y;
    return *this;
  }
private:
  int y;
};

// Recommended practice is to declare "= default", if the default supplied behaviors are desired
// Understand clearly state your intent

class Base {
public:
  virtual ~Base() = default;                // make dtor virtual

  Base() {
    std::cout << "Base default ctor\n";
  }

  // To test out the 'move if possible' behavior on moving an object containing another whose move is deleted.
  // Turns out with default, NoMove default ctor is called again when move happens on Base.
  // With the commented out, NoMove copycon is called as expected.
  Base(Base&& rhs) = default;
  /*
   : x(std::move(rhs.x)),
     nm(rhs.nm) {
    std::cout << "Base movecon\n";
  }
  */

  Base& operator=(Base&& rhs) {
    std::cout << "Base move assignment opr\n";
    x = std::move(rhs.x);
    return *this;
  }

  Base(const Base&) {
    std::cout << "Base copy assignment\n";
  }

  Base& operator=(const Base&) = default;

private:
  int x;
  NoMove nm;
};

int main() {
  Base b{std::move(Base())};
  return 0;
}