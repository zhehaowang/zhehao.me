#include <iostream>
#include <string>

class StringWrapper {
  public:
    StringWrapper(const char *in) : d_data(in) {}

    const char& operator[](std::size_t idx) const {
        std::cout << "call on const version\n";
        return d_data[idx];
    }
    char& operator[](std::size_t idx) {
        std::cout << "call on non-const version\n";
        return d_data[idx];
    }
  private:
    std::string d_data;
};

class CTextBlock {
  public:
    CTextBlock(char *in) : d_data(in) {}

    char& operator[](std::size_t position) const   // inappropriate (but bitwise
    { return d_data[position]; }                   // const) declaration of
                                                   // operator[]
    char *d_data;
};

class TextBlock {
  public:
    TextBlock(const char *in) : d_data(in) {}
    const char& operator[](std::size_t position) const     // same as before
    {
        std::cout << "const version of operator[]\n";
        return d_data[position];
    }

    char& operator[](std::size_t position)         // now just calls const op[]
    {
    return
      const_cast<char&>(                         // cast away const on
                                                 // op[]'s return type;
        static_cast<const TextBlock&>(*this)     // add const to *this's type;
          [position]                             // call const version of op[]
      );
    }

    std::string d_data;
};

int main() {
    StringWrapper sw("Hello");
    std::cout << sw[0] << "\n";                   // calls non-const

    const StringWrapper csw("World");
    std::cout << csw[0] << "\n";                  // calls const

    sw[0] = 'Y';
    std::cout << sw[0] << "\n";                   // calls non-const

    char* content = "Hello";

    const CTextBlock cctb(content);        // declare constant object
    std::cout << cctb.d_data << "\n";      // calls non-const

    char *pc = &cctb[0];                   // call the const operator[] to get a
                                           // pointer to cctb's data

    //*pc = 'J';                             // cctb now has the value "Jello"
                                           // this results in 'bus error' on OSX
    //std::cout << cctb.d_data << "\n";

    TextBlock textBlock("Good");
    textBlock[0] = 'f';
    std::cout << textBlock.d_data << "\n";
    return 0;
}