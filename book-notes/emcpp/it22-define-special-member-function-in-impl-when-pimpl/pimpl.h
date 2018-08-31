#ifndef INCLUDED_PIMPL_H
#define INCLUDED_PIMPL_H

#include <memory>

class Widget {
  public:
    Widget();
    void doStuff() const;
  private:
    struct Impl;
    std::unique_ptr<Impl> p_impl;
};

#endif