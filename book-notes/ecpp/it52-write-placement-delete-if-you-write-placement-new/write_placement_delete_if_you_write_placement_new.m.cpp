#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <new>

// demonstrates placement news and deletes, the necessity of pairing them, and
// how not to have them hide the global default versions

class MyClass {
public:
  MyClass() { throw std::runtime_error("this will leak with placement new"); }

  // this is a `placement new`, in the sense that it takes extra arguments than
  // the global one.
  // there is a global placement new as well, taking a pointer of where the
  // object should be located.
  // the term `placement new` is overloaded in the sense that when people refer
  // to it, they usually mean the latter.
  static void* operator new(std::size_t   size,
                            std::ostream& out) throw(std::bad_alloc) {
    out << "MyClass class placement new\n";
    // always fall back to global one, no need to check if the size matches here
    return ::operator new(size);
  }

  // without a placement delete, as this ctor throws, the runtime system will
  // not be able to free memory allocated by the placement new.
private:
  int d_data;
};

class Base {
public:
  Base() {
    // throw std::runtime_error("this will not leak with placement new");
  }

  static void* operator new(std::size_t   size,
                            std::ostream& out) throw(std::bad_alloc) {
    out << "Base class placement new (custom)\n";
    return ::operator new(size);
  }

  // a matching operator delete needs to be present
  static void operator delete(void *ptr,
                              std::ostream& out) throw() {
    out << "Base class placement delete (called by C++ runtime system)\n";
    return ::operator delete(ptr);
  }

  // we'll need a non-placement operator delete as well, as the placement one
  // hides the global one
  // user calls on `delete pb` will hit this version
  static void operator delete(void *ptr) throw() {
    std::cout << "Base class regular dtor (custom)\n";
    return ::operator delete(ptr);
  }

  // similarly, to not hide the global operator new (due to the presence of our
  // our placement new), we need to declare a global version here, unless you
  // want to limit your clients to use the placement new you provided
  static void* operator new(std::size_t size) throw(std::bad_alloc) {
    std::cout << "Base class regular new (custom)\n";
    return ::operator new(size);
  }

  // the other two versions of new that exist globally are the placement new
  // (with void* parameter), and nothrow() new.
  // for brevity we don't show them here.
private:
  char d_char;
};

int main() {
  try {
    MyClass* ptr = new (std::cout) MyClass();
  } catch (const std::runtime_error& ex) {
    std::cout << "Object of MyClass would leak if no matching placement delete"
              << "\n";
    // valgrind: definitely lost 4 bytes, due to missing matching placement
    // delete
  }

  try {
    Base* pb = new (std::cout) Base;
    if (pb) {
      delete pb;
    }
  } catch (const std::runtime_error& ex) {
    // swallow
  }

  try {
    Base* pb1 = new Base;
    if (pb1) {
      delete pb1;
    }
  } catch (const std::runtime_error& ex) {
    // swallow
  }

  return 0;
}
