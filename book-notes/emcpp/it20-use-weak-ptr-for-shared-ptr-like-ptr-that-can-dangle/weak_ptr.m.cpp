#include <iostream>
#include <string>
#include <unordered_map>

// demonstrates one use case of weak_ptr in building a cached load operation, in
// which cache uses a weak_ptr that can dangle (to avoid hogging the space if we
// use shared_ptr), and let the client decide the lifetime of the objects
// returned by cache.

class Widget {
public:
  Widget() = default;
  Widget(const Widget&) = default;
  Widget(Widget&&) = default;
  Widget(int id) : d_x(id) {};

  Widget& operator=(const Widget&) = default;
  Widget& operator=(Widget&&) = default;
private:
  int d_x;
};

std::unique_ptr<const Widget> buildWidget(int id) {
  std::cout << "build widget " << id << "\n";
  return std::make_unique<const Widget>(id);
}

std::shared_ptr<const Widget> fastLoadWidget(int id) {
  std::cout << "fast load widget " << id << "\n";
  static std::unordered_map<int,
                            std::weak_ptr<const Widget>> cache;

  auto objPtr = cache[id].lock();   // objPtr is std::shared_ptr
                                    // to cached object (or null
                                    // if object's not in cache)

  if (!objPtr) {                    // if not in cache,
    objPtr = buildWidget(id);       // build it
    cache[id] = objPtr;             // cache it
  }
  return objPtr;
}

int main() {
  std::shared_ptr<const Widget> wp = fastLoadWidget(3);
  auto wp1 = fastLoadWidget(3);
  wp  = nullptr;
  wp1 = nullptr;
  auto wp2 = fastLoadWidget(3);
  return 0;
}