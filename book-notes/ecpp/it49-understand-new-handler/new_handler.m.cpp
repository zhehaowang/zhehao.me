#include <iostream>
#include <string>
#include <memory>
#include <new>

// demonstrates customizing set_new_handler (the function new calls before
// throwing, upon being unable to satisfy a memory allocation request), and
// making a template that would allow easy customization of new_handler for that
// class (using curiously recurring template pattern)

void outOfMem() {
  // demonstrates how this is called repeatedly (in an attempt to free up memory
  // using this call)
  static int globalCounter = 0;
  if (globalCounter < 3) {
    std::cerr << "Custom: unable to satisfy request for memory\n";
  } else {
    std::set_new_handler(nullptr);
  }
  ++globalCounter;
}

void clientOutOfMem() {
  static int customGlobalCounter = 0;
  if (customGlobalCounter < 3) {
    std::cerr << "Custom: object allocation out of memory (object customized)"
              << "\n";
  } else {
    abort();
  }
  ++customGlobalCounter;
}

// RAII class for resetting new_handler
class NewHandlerHolder {
public:
  explicit NewHandlerHolder(std::new_handler nh)    // acquire current
   : handler(nh) {}                                 // new-handler

  ~NewHandlerHolder()                               // release it
  { std::set_new_handler(handler); }

  NewHandlerHolder(const NewHandlerHolder&) = delete;
  NewHandlerHolder& operator=(const NewHandlerHolder&) = delete;
private:
  std::new_handler handler;                         // remember it
};

// templated base to support a class that enables customizing class-specific
// new handler
template <typename T>             // "mixin-style" base class for
class NewHandlerSupport {         // class-specific set_new_handler
public:                           // support
  static std::new_handler set_new_handler(std::new_handler p) throw();
  static void * operator new(std::size_t size) throw(std::bad_alloc);

  // ...                          // other versions of op. new —
                                  // see Item 52
private:
  static std::new_handler currentHandler;
};

template <typename T>
std::new_handler
NewHandlerSupport<T>::set_new_handler(std::new_handler p) throw() {
  std::new_handler oldHandler = currentHandler;
  currentHandler = p;
  return oldHandler;
}

template<typename T>
void* NewHandlerSupport<T>::operator new(std::size_t size)
  throw(std::bad_alloc) {
  NewHandlerHolder h(std::set_new_handler(currentHandler));
  return ::operator new(size);
}

// this initializes each currentHandler to null
template<typename T>
std::new_handler NewHandlerSupport<T>::currentHandler = 0;

// client class
// this cannot be private inheritance, since otherwise everything inside
// NewHandlerSupport, set_new_handler included, will be private. We won't be
// able to do Widget::set_new_handler(...). We can use a different syntax
// NewHandlerSupport<Widget>::set_new_handler(...) but arguably more obscure.
class Widget : public NewHandlerSupport<Widget> {
public:
  Widget() = default;
private:
  int p_data[100000000000000L];
};

int main() {
  std::set_new_handler(outOfMem);
  try {
    int *pBigDataArray = new int[100000000000000L];
  } catch (const std::bad_alloc& ex) {
    // swallow
  }


  Widget::set_new_handler(0);        // set the Widget-specific
                                     // new-handling function to
                                     // nothing (i.e., null)
  try {
    Widget *pw2 = new Widget;        // if mem. alloc. fails, throw an
                                     // exception immediately. (There is
                                     // no new- handling function for
                                     // class Widget.)
  } catch (const std::bad_alloc& ex) {
    // swallow
  }
  
  Widget::set_new_handler(clientOutOfMem); // set outOfMem as Widget's
                                           // new-handling function

  Widget *pw1 = new Widget;          // if memory allocation
                                     // fails, call outOfMem
  return 0;
}
