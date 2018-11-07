#include <iostream>
#include <string>
#include <memory>
#include <new>

// demonstrates global and class-specific new that conforms to the conventional
// behaviors 

// a conformant `operator new` needs to
//  * account for zero-allocation
//  * call new_handler in a loop
//  * throw std::bad_alloc if null handler
// (or calling a conformant one)
void* operator new(std::size_t size) throw(std::bad_alloc) {
  std::cout << "custom global operator new\n";

  if (size == 0) {
    size = 1;
  }

  while (true) {
    void* handle = malloc(size);
    if (handle) {
      return handle;
    } else {
      std::new_handler currentHandler = std::get_new_handler(); // since c++11
      if (currentHandler) {
        (*currentHandler)();
      } else {
        throw std::bad_alloc();
      }
    }
  }
}

class Base {
public:
  Base() : d_data(10) {}
  virtual ~Base() = default;

  static void* operator new(std::size_t size) throw(std::bad_alloc) {
    std::cout << "Base class operator new\n";

    if (size != sizeof(Base)) {
      std::cout << "Allocation not of sizeof(Base), falling back to default\n";
      return ::operator new(size);
    }

    // in this custom new we don't do anything in addition
    return ::operator new(size);
  }

  void print() const { std::cout << d_data << "\n"; }
private:
  int d_data;
};

class Derived : public Base {
public:
private:
  char d_char;
};

int main() {
  double *pData = new double;
  delete pData;

  Base* pd = new Derived;
  delete pd;

  Base* pb = new Base;
  delete pb;

  // it would seem without an `operator new[] overload`, the global default is
  // used with no issues
  Base* pds = new Base[4];
  pds[3].print();
  delete[] pds;
  return 0;
}
