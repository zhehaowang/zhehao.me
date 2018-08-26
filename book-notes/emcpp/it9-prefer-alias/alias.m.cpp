#include <iostream>
#include <string>
#include <list>

template <typename T>
using MyList = std::list<T>;

//template <typename T>
//typedef std::list<T> MyListType;
// compiler error: a typedef cannot be a template!

int main() {
  MyList<int> list1;
  //MyList<int> list2;
  return 0;
}