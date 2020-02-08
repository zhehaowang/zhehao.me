#include <iostream>
#include <string>
#include <thread>
#include <atomic>

//
// Peterson's Algorithm.
// 
// Synchronization (of two threads) without locks, or atomic xchg,
// compare_and_set, test_and_swap.
// Imagine we only have `load` and `store` and our memory model satisfies
// sequential consistency. (With relaxed memory model as-is common case
// with modern hardware, memory fence would be needed to prevent hardware
// `load` and `store` reordering. Refer to notes in Performance Engineering)
//

class CriticalObject {
  public:
    void increment() { ++count; }
    void decrement() { --count; }
    int get() const { return count; }

  private:
    int count = 0;
};

struct Control {
    // don't optimize this to registers
    volatile bool incWants = false;
    volatile bool decWants = false;
    volatile char turn = 0;
};

void incrementJob(CriticalObject& o, Control& ctrl, int times) {
    for (int c = 0; c < times; ++c) {
        ctrl.incWants = true;
        ctrl.turn = 'd';
        std::atomic_thread_fence(std::memory_order_release);
        while (ctrl.decWants && ctrl.turn == 'd') {};
        // compiler fence
        asm volatile ("" ::: "memory");
        o.increment();
        asm volatile ("" ::: "memory");
        ctrl.incWants = false;
    }
}

void decrementJob(CriticalObject& o, Control& ctrl, int times) {
    for (int c = 0; c < times; ++c) {
        ctrl.decWants = true;
        ctrl.turn = 'i';
        std::atomic_thread_fence(std::memory_order_release);
        while (ctrl.incWants && ctrl.turn == 'i') {};
        asm volatile ("" ::: "memory");
        o.decrement();
        asm volatile ("" ::: "memory");
        ctrl.decWants = false;
    }
}

// still doesn't quite work...
int main() {
    Control c;
    CriticalObject o;
    std::thread tInc(incrementJob, std::ref(o), std::ref(c), 12000);
    std::thread tDec(decrementJob, std::ref(o), std::ref(c), 12001);
    tInc.join();
    tDec.join();
    std::cout << "final value is " << o.get() << "\n";
    return 0;
}