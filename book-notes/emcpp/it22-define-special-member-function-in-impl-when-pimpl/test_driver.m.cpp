#include <pimpl.h>

// demonstrates a case when using unqiue_ptr for pimpl, we need to define dtor
// in the implementation file; shows copy / move impl for pimpl classes using
// unique_ptr

int main() {
  Widget w;
  // The dtor of std::unique_ptr will complain about sizeof or delete to an
  // incomplete type.
  // Problem is that as compiler generates code for w's deletion (delete on the
  // raw pointer inside unique_ptr),
  // delete needs to be called on a complete type. In this translation unit with
  // pimpl.h included, struct Impl
  // is not a complete type.
  w.doStuff();
  return 0;
}