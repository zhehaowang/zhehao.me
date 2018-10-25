#include <iostream>
#include <string>
#include <mutex>
#include <memory>

// demonstrates offering strong exception safety using copy-and-swap and objects
// to manage resources (along with a pimpl)

struct MyClassImpl {
  // this pimpl class offers no encapsulation, perceived as unnecessary
  MyClassImpl(const std::string& data, int counter)
   : d_data(data), d_counter(counter) {}

  MyClassImpl(const MyClassImpl& rhs) = default;
  MyClassImpl& operator=(const MyClassImpl& rhs) = default;

  std::string d_data;
  int         d_counter;
};

class MyClass {
public:
  MyClass(const std::string& data, int counter)
   : d_impl(new MyClassImpl(data, counter)) {}

  MyClass& operator=(const MyClass& rhs) {
    *d_impl = *(rhs.d_impl);
    return *this;
  }
  
  MyClass(const MyClass& rhs) : d_impl(std::make_unique<MyClassImpl>(*(rhs.d_impl))) {}

  void updateData(const std::string& data) {
    // this function offers strong exception safety (transactional, the whole
    // thing either happened or it didn't)
    using std::swap;

    std::lock_guard<std::mutex> guard(d_mutex);
    
    std::unique_ptr<MyClassImpl> pCopy = std::make_unique<MyClassImpl>(data, d_impl->d_counter);
    swap(d_impl, pCopy);
    d_impl->d_counter += 1;

    return;
  }

  int counter() const { return d_impl->d_counter; }
  std::string data() const { return d_impl->d_data; }
private:
  std::unique_ptr<MyClassImpl> d_impl;
  std::mutex  d_mutex;
  // any preference for mutex going into impl vs not? Book had it this way tho
  // due to copying concerns potentially?
};

int main() {
  std::string data("data");
  MyClass my(data, 0);

  std::string data1("data1");
  my.updateData(data1);
  my.updateData(data1);
  std::cout << my.data() << " " << my.counter() << "\n";
  return 0;
}