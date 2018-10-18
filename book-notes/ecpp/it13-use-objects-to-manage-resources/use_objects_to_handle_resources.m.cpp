#include <iostream>
#include <string>

// demonstrates the pitfall of calling raw deletes

class ObjectToBeCreatedWithFactoryMethod {};

// don't design your interface like this, refer to item 18
ObjectToBeCreatedWithFactoryMethod* createObject() {
  // expects the client to delete the allocated object
  return new ObjectToBeCreatedWithFactoryMethod();
}

void iCouldThrow() {
  throw std::runtime_error("blow up");
}

void func() {
  ObjectToBeCreatedWithFactoryMethod* pObj = createObject();
  iCouldThrow();
  // using delete directly here is a bad idea, if the above throws
  // or returns prematurely (think how the code evolves over time),
  // we leak resources.
  delete pObj;
}

void func1() {
  std::unique_ptr<ObjectToBeCreatedWithFactoryMethod> pObj(createObject());
  iCouldThrow();
}

int main() {
  try {
    func();  // leaks
    func1(); // does not leak
  } catch (const std::runtime_error& e) {
    // swallow
  }
  return 0;
}