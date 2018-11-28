#include <iostream>
#include <string>

// demonstrates in a composition, default move con of the whole uses the default
// ctor of the part, if the part has its move con disabled. (Shouldn't the 
// behavior of part be use copy con if no move con in this case though?)

class NoMove {
public:
  NoMove() {
    std::cout << "NoMove default ctor\n";
  };
  
  NoMove(int y) : d_y(y) {
    std::cout << "NoMove ctor taking in y\n";
  };

  ~NoMove() = default;
  
  NoMove(NoMove&&) = delete;
  /*
  {
    std::cout << "NoMove move ctor\n";
  } */

  NoMove& operator=(NoMove&&) = delete;

  NoMove(const NoMove& rhs) : d_y(rhs.d_y) {
    std::cout << "NoMove copycon\n";
  }

  NoMove& operator=(const NoMove& rhs) {
    std::cout << "NoMove copy assignment\n";
    d_y = rhs.d_y;
    return *this;
  }

  void setY(int y) { d_y = y; }
  int y() const { return d_y; }
private:
  int d_y;
};

// Recommended practice is to declare "= default", if the default supplied
// behaviors are desired
// Clearly state your intent

class Base {
public:
  virtual ~Base() = default;                // make dtor virtual

  Base() {
    std::cout << "Base default ctor\n";
  }

  Base(const NoMove& nm) : d_nm(nm) {
    std::cout << "Base ctor taking in NoMove\n";
  }

  // To test out the 'move if possible' behavior on moving an object containing
  // another whose move is deleted.
  // Turns out with default, NoMove default ctor is called again when move
  // happens on Base.
  // With the commented out, NoMove copycon is called as expected.
  Base(Base&& rhs) = default;
  /*
   : x(std::move(rhs.x)),
     d_nm(rhs.d_nm) {
    std::cout << "Base movecon\n";
  }
  */

  Base& operator=(Base&& rhs) {
    std::cout << "Base move assignment opr\n";
    x = std::move(rhs.x);
    return *this;
  }

  Base(const Base&) {
    std::cout << "Base copy con\n";
  }

  Base& operator=(const Base&) = default;

  const NoMove& nm() const { return d_nm; }
private:
  int x;
  NoMove d_nm;
};

int main() {
  // Q1: the std::move shouldn't matter here in the first place, the param is a
  // temporary, and the cast should be no-op. But with / without it the printed
  // result is 0 / 10.
  // Q2: in the version with std::move, and Base's default move con is called,
  // shouldn't the default move con use NoMove's copy con, since NoMove's move
  // con is disabled? In fact it uses NoMove's default ctor.
  // Q3: in the version without std::move, why is Base's movecon not called?
  // A3: presumably optimized away by compiler: this behavior is present with
  // -O0
  Base b(std::move(Base(NoMove(10))));
  //Base b(Base(NoMove(10)));
  std::cout << b.nm().y() << "\n";
  return 0;
}