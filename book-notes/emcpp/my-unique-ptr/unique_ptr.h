#ifndef INCLUDED_UNIQUE_PTR
#define INCLUDED_UNIQUE_PTR

// TODO: support custom deleters
// TODO: make_unique struct
// TODO: threadsafety note

#include <memory>

template <typename T>
class UniquePtr {
  public:
    // noexcept or exception-neutral?
    UniquePtr() noexcept : d_ptr(nullptr) {}

    // generalized move ctor: it45 from ec++, so that we can do
    // UniquePtr<Derived> pd;
    // UniquePtr<Base>    pb = pd;
    // TODO: should we use std::forward on this universal reference, why or why
    // not?
    template <typename U>
    UniquePtr(U&& rhs) noexcept : d_ptr(std::forward<U>(rhs).release()) {}

    virtual ~UniquePtr() {
        if (d_ptr) {
            // TODO: differentiate [] and delete; delete func template
            delete d_ptr;
        }
    }

    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& rhs) noexcept : d_ptr(rhs.release()) {}

    /*
    // turns out this is shadowed by the universal version
    UniquePtr& operator=(UniquePtr&& rhs) noexcept {
        if (rhs.isNull() || !isNull()) {
            std::cout << "regular move called\n";
            reset();
        }
        d_ptr = rhs.release();
    }
    */

    template <typename U>
    UniquePtr& operator=(U&& rhs) noexcept {
        if (rhs.isNull() || !isNull()) {
            reset();
        }
        d_ptr = std::forward<U>(rhs).release();
        return *this;
    }

    explicit UniquePtr(T* ptr) noexcept : d_ptr(ptr) {}

    T* get() const noexcept {
        return d_ptr;
    }

    bool isNull() const noexcept {
        return d_ptr == nullptr;
    }

    // Does a unique_ptr take ownership from the raw pointer it's given?
    // if so, it'd be
    //void reset(T*&& ptr) noexcept
    // ...
    // ptr = nullptr
    // ...
    void reset(T* ptr) noexcept {
        if (d_ptr) {
            delete d_ptr;
        }
        d_ptr = ptr;
    }

    void reset() noexcept {
        if (d_ptr) {
            delete d_ptr;
        }
    }

    T* release() noexcept {
        T* temp = d_ptr;
        d_ptr = nullptr;
        return temp;
    }

    T& operator*() const noexcept {
        return *d_ptr;
    }

    T* operator->() const noexcept {
        return d_ptr;
    }
  private:
    T* d_ptr;
};

#endif