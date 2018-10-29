#include <iostream>
#include <string>

// demonstrates pure virtual (with a definition), simple virtual, and non
// virtual

class Shape {
public:
  ~Shape() = default;
  
  virtual void draw() = 0;
    // pure virtual: child only inherits interface and have to provide impl
  
  virtual void error() { std::cout << "Shape::error()\n"; }
    // simple virtual: child inherits interface and could choose to inherit impl
  
  void objectId() { std::cout << "Shape::objectId() (invariant)\n"; }
    // non virtual: invariant in the chain and should not be redefined. Child
    // inherits both interface and impl
};

void Shape::draw() {
  // pure virtual functions can have a default impl, which can be used to convey
  // to child / clients "you may inherit this impl only by explicitly specifying
  // so"
  std::cout << "Shape::draw() (default impl for a pure virtual function)\n";
}

class Rectangle : public Shape {
public:
  virtual void draw() { std::cout << "Rectangle::draw()\n"; }
};

class Circle : public Shape {
public:
  virtual void draw() { Shape::draw(); }

  virtual void error() { std::cout << "Circle::error()\n"; }
};

int main() {
  Shape* ps = new Rectangle();
  ps->draw();
  ps->error();
  ps->objectId();

  Shape* ps1 = new Circle();
  ps1->draw();
  ps1->error();
  ps1->objectId();
  return 0;
}