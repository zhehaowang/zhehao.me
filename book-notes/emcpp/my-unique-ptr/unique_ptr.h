#ifndef INCLUDED_UNIQUE_PTR
#define INCLUDED_UNIQUE_PTR

// TODO: support custom deleters
// TODO: make_unique struct
// TODO: threadsafety note

template <typename T>
class UniquePtr {
  public:
    // noexcept or exception-neutral?
    UniquePtr() noexcept : d_ptr(nullptr) {}

    virtual ~UniquePtr() {
        if (d_ptr) {
            // TODO: differentiate [] and delete; delete func template
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

    // Similar question as reset. Take universal reference or value?
    UniquePtr(T* ptr) noexcept : d_ptr(ptr) {}

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