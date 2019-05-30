#include <iostream>
#include <string>
#include <memory>

class Owner {
public:
    std::unique_ptr<int> x;
};

std::shared_ptr<int> makeShared(std::shared_ptr<int> x) {
    return x;
}

int main() {
    Owner o;
    auto s = makeShared(std::move(o.x));
    return 0;
}