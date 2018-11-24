#include <iostream>
#include <string>
#include <vector>

// demonstrates undefined behavior when creating two shared pointers from the
// same raw pointer.
// double free caused by having two control blocks with count 1 but only one
// instance of data, first case by a pointer to this, second case by pointer to
// data.

class Widget : public std::enable_shared_from_this<Widget> {
public:
  void process(std::vector<std::shared_ptr<Widget>>& processedWidgets) {
    processedWidgets.emplace_back(this);    // add it to list of
                                            // processed Widgets;
                                            // this is wrong!
  }

  void processCorrect(std::vector<std::shared_ptr<Widget>>& processedWidgets) {
    processedWidgets.emplace_back(shared_from_this());
  }
private:
  int d_x;
};

int main() {
  {
    // auto wp = std::make_shared<Widget>();
    // std::vector<std::shared_ptr<Widget>> processedWidgets;
    // wp->process(processedWidgets);
    // wp->process(processedWidgets);
    // double free without shared_from_this
  }

  {
    auto wp = std::make_shared<Widget>();
    // if Widget is not held by a shared_ptr, it'll just throw:
    //   type std::__1::bad_weak_ptr: bad_weak_ptr
    // hence why we typically have a creator function for classes that inherit
    // from shared_from_this, and disable their ctors.
    std::vector<std::shared_ptr<Widget>> processedWidgets;
    wp->processCorrect(processedWidgets);
    wp->processCorrect(processedWidgets);
    // all good
  }

  {
    auto loggingDel = [](int *pw) {  // custom deleter
      std::cout << "dtor\n";         // (as in Item 18)
      delete pw;
    };

    auto pw = new int(5);
    std::shared_ptr<int> spw1(pw, loggingDel);  // create control
                                                // block for *pw

    std::shared_ptr<int> spw2(pw, loggingDel);  // create 2nd
                                                // control block
                                                // for *pw!
    // double free when freeing spw1 and spw2
  }
  return 0;
}