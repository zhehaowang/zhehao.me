#include <iostream>
#include <string>
#include <memory>
#include <new>

// demonstrates a custom albeit unideal `operator new`

static const int signature = 0xDEADBEEF;

typedef unsigned char Byte;

// this code has several flaws: not conforming to convention (e.g. call handler
// in a loop), and alignment may be broken (returning a pointer from malloc is
// safe, but offset this by an int may break alignment assumption for doubles)
void* operator new(std::size_t size) throw(std::bad_alloc) {
  using namespace std;
  cout << "using custom new\n";

  size_t realSize = size + 2 * sizeof(int);    // increase size of request so2
                                               // signatures will also fit inside

  void *pMem = malloc(realSize);               // call malloc to get theactual
  if (!pMem) throw bad_alloc();                // memory

  // write signature into first and last parts of the memory
  *(static_cast<int*>(pMem)) = signature;
  *(reinterpret_cast<int*>(static_cast<Byte*>(pMem) + realSize - sizeof(int))) =
    signature;

  // return a pointer to the memory just past the first signature
  return static_cast<Byte*>(pMem) + sizeof(int);
}

int main() {
  double *pData = new double;
  *pData = 10.0;
  std::cout << *pData << "\n";
  return 0;
}
