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
        return std::make_unique<Iter>(this);
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

    VectorIterator(Container* _container)
      : container(_container), index(-1) {}

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
private:
    Container* container;
    int index;
};

int main() {
    VectorContainer<int> c;
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