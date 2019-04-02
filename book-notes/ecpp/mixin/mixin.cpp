#include <string>
#include <iostream>

// An example of mixin, largely from example here:
// https://stackoverflow.com/questions/18773367/what-are-mixins-as-a-concept
// According to which one use case for mixins is when we have orthogonal
// (primitive, self-sufficient) logic components and we want to combine them
// to model more complex logic.

// Ultimately this looks like inheritance from a template base in order to
// decorate it, base is templated so that we can more concisely / flexibly glue
// logical components.

template <typename V>
class MyValue {
  public:
    using value_type = V;

    MyValue(V x) : _x(x) {}

    // note how we don't mark this virtual as hiding is intended: our usage of
    // this requires no dynamic binding.
    // we require virtual only if we wish to pass derived to pointer to base and
    // expect behavior of derived.
    void set(V x) {
        _x = x;
    }

    V get() const {
        return _x;
    }
  private:
    V _x;
};

// this looks like a decorator pattern where we derive from base and have a
// reference to it (not quite), we decorate set() method with extra operation
// in order to support undo().
template <typename Base, typename V = typename Base::value_type>
class UndoableNumber : public Base {
  public:
    UndoableNumber(V x) : Base(x) {};

    void undo() {
        Base::set(before);
    }

    void set(V newVal) {
        before = Base::get();
        Base::set(newVal);
    }
  private:
    V before;
};

template <typename Base, typename V = typename Base::value_type>
class RedoableNumber : public Base {
  public:
    RedoableNumber(V x) : Base(x) {};

    void redo() {
        Base::set(after);
    }

    void set(V newVal) {
        after = newVal;
        Base::set(newVal);
    }

    // an alternative of implementing this was hide undo():
    // void undo() {
    //     after = Base::get();
    //     Base::undo();
    // }
    // this expects undo() in Base though Redo is only applicable in the case of
    // having undo.
  private:
    V after;
};

int main() {
    using UndoRedoableNumber = RedoableNumber<UndoableNumber<MyValue<int>>>;
    UndoRedoableNumber x(12);
    std::cout << "init 12: " << x.get() << "\n";
    x.set(10);
    std::cout << "set 10:  " << x.get() << "\n";
    x.undo();
    std::cout << "undo:    " << x.get() << "\n";
    x.redo();
    std::cout << "redo:    " << x.get() << "\n";
    x.set(8);
    std::cout << "set 8:   " << x.get() << "\n";
    x.undo();
    std::cout << "undo:    " << x.get() << "\n";
    x.redo();
    std::cout << "redo:    " << x.get() << "\n";
    return 0;
}