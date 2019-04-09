#include <iostream>
#include <string>
#include <vector>
#include <memory>

// todo: make polymorphic, make non-trivial

template <typename T>
class VectorIterator;

template <typename T>
class VectorContainer {
public:
    using Iter = VectorIterator<T>;
    ~VectorContainer() = default;
    
    std::unique_ptr<Iter> makeIterator() {
        // return std::make_unique<Iter>(this);
        return std::unique_ptr<Iter>(new Iter(this));
    }

    void add(const T& elem) {
        elems.push_back(elem);
    }
    
    template<typename U>
    friend class VectorIterator;
private:
    std::vector<T> elems;
};

template <typename T>
class VectorIterator {
public:
    using Container = VectorContainer<T>;

    T& getElem() {
        return container->elems[index];
    }
    void start() {
        index = 0;
    }
    bool hasNext() const {
        return index >= 0 && index < container->elems.size();
    }
    void next() {
        index += 1;
    }

    template<typename U>
    friend class VectorContainer;
    
    // while this would make make_unique work, it doesn't really prevent client
    // usage of the ctor as client can use it via make_unique. In this case to
    // hide the ctor probably best is to not use make_unique and use raw new, as
    // done above

    // friend std::unique_ptr<VectorIterator> std::make_unique<VectorIterator>(Container*&);
    // friend std::unique_ptr<VectorIterator> std::make_unique<VectorIterator>(Container*&&);
private:
    VectorIterator(Container* _container)
      : container(_container), index(-1) {}

    Container* container;
    int index;
};

// defining this out of line looks something like this, though defining this out
// of line is not needed
// template <typename T>
// inline std::unique_ptr<typename VectorContainer<T>::Iter> VectorContainer<T>::makeIterator() {
//     return std::make_unique<Iter>(this);
// }

int main() {
    VectorContainer<int> c;
    
    // VectorIterator<int> d(&c);
    // std::make_unique<VectorIterator<int>>(&c);

    c.add(3);
    c.add(4);
    c.add(5);
    auto it = c.makeIterator();
    it->start();
    std::cout << it->getElem() << "\n";
    std::cout << "\n";
    while (it->hasNext()) {
        std::cout << it->getElem() << "\n";
        it->next();
    }
    return 0;
}