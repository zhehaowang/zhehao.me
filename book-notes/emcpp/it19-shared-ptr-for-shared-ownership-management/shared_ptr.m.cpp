#include <iostream>
#include <string>

const auto CREATE_TYPE = 1;

class Widget {
public:
private:
  int d_x;
};

int main() {
  auto loggingDel = [](Widget *pw)        // custom deleter
                  {                     // (as in Item 18)
                    std::cout << "dtor\n";
                    delete pw;
                  };

  auto pw = new Widget();
  std::shared_ptr<Widget> spw1(pw, loggingDel);  // create control
                                                 // block for *pw

  std::shared_ptr<Widget> spw2(pw, loggingDel);  // create 2nd
                                                 // control block
                                                 // for *pw!
  // double free
  return 0;
}