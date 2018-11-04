#ifndef INCLUDED_SHARED_PTR
#define INCLUDED_SHARED_PTR

#include <atomic>

class ControlBlock {
  public:
    ControlBlock() : d_reference_cnt(1) {}
    virtual ~ControlBlock() = default;

    ControlBlock(const ControlBlock&) = default;
    ControlBlock& operator=(const ControlBlock&) = default;

    ControlBlock(ControlBlock&&) = default;
    ControlBlock& operator=(ControlBlock&&) = default;

    void incrementReference() noexcept {
        d_reference_cnt ++;
    }

    bool decrementReference() {
        if (d_reference_cnt == 0) {
            throw std::runtime_error("decrement called on reference cnt 0");
        }
        d_reference_cnt --;
        return d_reference_cnt == 0;
    }
  private:
    // TODO: custom allocator support
    // TODO: custom deleter support
    std::atomic<int> d_reference_cnt;
    // TODO: weak pointer support
    std::atomic<int> d_weak_cnt;
};

template <typename T>
class SharedPtr {
  public:
    SharedPtr() : d_ptr_p(nullptr), d_cb_p(nullptr) {}
    explicit SharedPtr(T* ptr) : d_ptr_p(ptr), d_cb_p(new ControlBlock()) {}

    SharedPtr(const SharedPtr& rhs) noexcept
     : d_ptr_p(rhs.d_ptr_p),
       d_cb_p(rhs.d_cb_p) {
        if (d_cb_p && d_ptr_p) {
            d_cb_p->incrementReference();
        }
    }

    // TODO: generalized copy / move cons, assignment opr
    /*
    // How to implement a generalized copy con for shared_ptr, without making
    // ControlBlock pointer public?
    //
    // Also differentiating generalized move con and generalized copy con:
    // generalized move con, once made template, will have its argument treated
    // as a universal reference, thus can be instantiated by lvalue calls,
    // although we want different behaviors for shared_ptr lvalue (copy), and
    // rvalue (move) calls. Right?

    template <typename U>
    SharedPtr(const U& rhs) noexcept
     : d_ptr_p(rhs.get()),
       d_cb_p(rhs.d_cb_p) {
        if (d_cb_p && d_ptr_p) {
            d_cb_p->incrementReference();
        }
    }
    */

    // TODO: use my unique ptr to build ctor(unique_ptr)

    SharedPtr& operator=(const SharedPtr& rhs) {
        if (d_ptr_p != rhs.d_ptr_p) {
            decrementAndFreeIfNeeded();
            d_ptr_p = rhs.d_ptr_p;
            d_cb_p  = rhs.d_cb_p;
            d_cb_p->incrementReference();
        }
        return *this;
    }

    SharedPtr(SharedPtr&&) = default;

    SharedPtr& operator=(SharedPtr&& rhs) {
        if (d_ptr_p != rhs.d_ptr_p) {
            decrementAndFreeIfNeeded();
            d_ptr_p = rhs.d_ptr_p;
            d_cb_p  = rhs.d_cb_p;
            rhs.d_cb_p  = nullptr;
            rhs.d_ptr_p = nullptr;
        }
        return *this;
    }

    virtual ~SharedPtr() {
        if (d_cb_p && d_ptr_p) {
            decrementAndFreeIfNeeded();
        }
    }

    // TODO: implement reset

    T* get() const noexcept {
        return d_ptr_p;
    }

    T& operator*() const noexcept {
        return *d_ptr_p;
    }

    T* operator->() const noexcept {
        return d_ptr_p;
    }
  private:
    void decrementAndFreeIfNeeded() {
        if (d_cb_p->decrementReference()) {
            delete d_ptr_p;
            delete d_cb_p;
        }
    }

    T*            d_ptr_p;
    ControlBlock* d_cb_p;
};

#endif