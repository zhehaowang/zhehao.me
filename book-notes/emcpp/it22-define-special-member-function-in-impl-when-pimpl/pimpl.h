#ifndef INCLUDED_PIMPL_H
#define INCLUDED_PIMPL_H

#include <memory>

class Widget {
  public:
    Widget();

    ~Widget();  // this dtor declaration cannot be omitted
    // we declare it here so that we don't run into issues deleting an
    // incomplete type (unique_ptr, whose deleter is part of the type), when
    // using Widget in a different translation unit.

    Widget(Widget&& rhs) noexcept;              // declarations
    Widget& operator=(Widget&& rhs) noexcept;   // only

    Widget(const Widget& rhs);              // declarations
    Widget& operator=(const Widget& rhs);   // only

    void doStuff() const;
  private:
    struct Impl;
    std::unique_ptr<Impl> p_impl;
};

#endif