#ifndef INCLUDED_UNIQUE_PTR
#define INCLUDED_UNIQUE_PTR

// TODO: support custom deleters
// TODO: make_unique struct

template <typename T>
class UniquePtr {
  public:
    // noexcept or exception-neutral?
    UniquePtr() noexcept : d_ptr(nullptr) {}

    virtual ~UniquePtr() {
        if (d_ptr) {
            delete d_ptr;
        }
    }

    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& rhs) noexcept {
        d_ptr = rhs.d_ptr;
        rhs.d_ptr  = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& rhs) noexcept {
        d_ptr = rhs.d_ptr;
        rhs.d_ptr = nullptr;
    }

    T* get() const noexcept {
        return d_ptr;
    }

    bool isNull() const noexcept {
        return d_ptr == nullptr;
    }

    void reset(T* ptr) noexcept {
        d_ptr = ptr;
        ptr = nullptr;
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