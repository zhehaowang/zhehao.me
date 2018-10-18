#include <iostream>
#include <string>

// demonstrates explicit and implicit approaches to expose resources
// managed by RAII classes

class FontHandle {};

FontHandle getFontHandle() {
  std::cout << "allocate FontHandle\n";
  return FontHandle();
}

void releaseFontHandle(FontHandle fh) {
  std::cout << "release FontHandle\n";
}

class Font {                           // RAII class
public:
  explicit Font(FontHandle fh)         // acquire resource;
  : f(fh)                              // use pass-by-value, because the
  {}                                   // C API does

  ~Font() { releaseFontHandle(f); }          // release resource

  // explicit
  FontHandle get() const {
    std::cout << "explicit get call!\n";
    return f;\
  }
  // explicit conversion function
  // downside is explicit calls to get() function is required everywhere
  // FontHandle is expected

  // alternative, implicit approach (note the operator overload!)
  operator FontHandle() const {
    std::cout << "implicit conversion!\n";
    return f;
  }
  // implicit conversion function
  // downside is increased chance of error
private:
  FontHandle f;                        // the raw font resource
};

void anotherAPIExpectingFontHandle(FontHandle fh) {}

int main() {
  Font font(getFontHandle());
  
  // implicit:
  anotherAPIExpectingFontHandle(font);

  // explicit:
  anotherAPIExpectingFontHandle(font.get());
  return 0;
}