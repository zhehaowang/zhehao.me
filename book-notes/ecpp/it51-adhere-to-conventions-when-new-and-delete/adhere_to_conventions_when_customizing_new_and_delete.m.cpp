#include <iostream>
#include <string>
#include <memory>
#include <new>

// demonstrates global and class-specific new and delete 

// a conformant `operator new` needs to
//  * account for zero-allocation
//  * call new_handler in a loop
//  * throw std::bad_alloc if null handler
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
  static void* operator new(std::size_t size) throw(std::bad_alloc) {
    std::cout << "Base class operator new\n";

    if (size != sizeof(Base)) {
      std::cout << "Allocation not of sizeof(Base), falling back to default\n";
      return ::operator new(size);
    }

    // in this custom new we don't do anything in addition
    return ::operator new(size);
  }
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
  return 0;
}
