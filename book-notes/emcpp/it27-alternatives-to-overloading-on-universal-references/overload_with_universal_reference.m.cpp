#include <iostream>
#include <string>
#include <vector>

// demonstrates alternatives to having a universal reference function override,
// one is enable_if, where you can specify the condition where this universal
// reference template can be applied.
// another is tagged dispatch, where you can have one universal reference
// version call different underlying impls based on conditions tested on the
// type with which the template is instantiated.

// enable_if:

class Widget {
public:
  Widget() = default;
  Widget(const Widget&) noexcept {
    std::cout << "base copy ctor\n";
  }
  Widget& operator=(const Widget&) noexcept {
    std::cout << "base copy assignment\n";
    return *this;
  }
  Widget(Widget&&) noexcept {
    std::cout << "base move ctor\n";
  }
  Widget& operator=(Widget&&) noexcept {
    std::cout << "base move assignment\n";
    return *this;
  }
  ~Widget() = default;

  // C++14
  template <typename T,
            typename = std::enable_if_t<
              !std::is_base_of<Widget, std::decay_t<T>>::value
               &&
              !std::is_integral<std::remove_reference_t<T>>::value
            >>
  Widget(T&& rhs) noexcept {
    std::cout << "base universal reference ctor\n";
  }
};

class ChildWidget : public Widget {
public:
  ChildWidget() = default;
  ChildWidget(const ChildWidget& rhs) : Widget(rhs) {
    std::cout << "child copy ctor\n";
  }
  ChildWidget(ChildWidget&& rhs) : Widget(std::move(rhs)) {
    std::cout << "child move ctor\n";
  }
};

// tag dispatch (we want different behaviors for integral and non-integral types)

template<typename T>                             // non-integral
void logAndAddImpl(T&& name, std::false_type)    // argument:
{                                                // treat as string
  std::vector<std::string> names;
  names.emplace(names.begin(), std::forward<T>(name));
  std::cout << "universal reference overload\n";
}

std::string nameFromIdx(int idx) {
  return "from an idx";
}

void logAndAddImpl(int idx, std::true_type)       // integral
{                                                 // argument: look
  std::vector<std::string> names;                 // up name and
  names.emplace(names.begin(), nameFromIdx(idx)); // call logAndAdd
  std::cout << "int overload\n";                  // with it
}                  

template<typename T>
void logAndAdd(T&& name)
{
  logAndAddImpl(
    std::forward<T>(name),
    std::is_integral<typename std::remove_reference<T>::type>()
  );
}


int main() {
// Tests enable_if:
  // Note now they call the copy ctor
  Widget w;
  auto w1(w);

  const Widget cw;
  auto cw1(cw);

  // Note now child copy ctor calls base's copy ctor
  ChildWidget child;
  auto child1(child);

  // And note the enable_if defined integral types out:
  // (meant to have a different overload)
  //short i = 3;
  //Widget w2(i);

// Tests tag dispatch
  logAndAdd("good");
  logAndAdd(1);

  return 0;
}