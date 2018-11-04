#include <iostream>
#include <string>
#include <memory>

// demonstrates cases where one can / should factor parameter code out of
// templates to avoid code bloat, by parameterizing a non-type parameter
// required to instantiate a template

// bloat version
template<typename T,           // template for n x n matrices of
         std::size_t n>        // objects of type T; a non-type parameter
class SquareMatrixBloat {      // on the size_t parameter
public:
  void invert() {              // invert the matrix in place
    // some business logic
    std::cout << "SquareMatrixBloat: invert\n";
  }
};

// unbloat version
template<typename T>
class SquareMatrixBase {
protected:
  SquareMatrixBase(std::size_t n, T *pMem)     // store matrix size and a
  : size(n), pData(pMem) {}                    // ptr to matrix values

  void setDataPtr(T *ptr) { pData = ptr; }     // reassign pData

  void invert(std::size_t matrixSize) {        // invert matrix of the given
    // some business logic                     // size. Work with 'pData' and
    std::cout << "SquareMatrixBase: invert "   // 'size'
              << matrixSize << "\n";
  }
private:
  std::size_t size;                            // size of matrix
  T *pData;                                    // pointer to matrix values
};

template<typename T, std::size_t n>
class SquareMatrix: private SquareMatrixBase<T> {
private:
  using SquareMatrixBase<T>::invert; // avoid hiding base version of invert,
                                     // item 33
public:
  SquareMatrix()                             // send matrix size and
  : SquareMatrixBase<T>(n, data) {}          // data ptr to base class
  // derived class is in charge actual allocation, but gives a pointer of its
  // data to the base so that invert knows what to work with

  void invert() { this->invert(n); }; // does this violate item 37? arguably not
                                      // as the inheritance relationship is not
                                      // public, you can't assign SquareMatrix*
                                      // to SquareMatrixBase* and call invert on
                                      // the same pointer and expect different
                                      // behaviors due to static binding.
                                      // SquareMatrixBase is not meant to be
                                      // exposed, by this design.
private:
  T data[n*n];
};


int main() {
  SquareMatrixBloat<double, 5> smb1;
  smb1.invert();

  SquareMatrixBloat<double, 10> smb2;
  smb2.invert();

  // business logic in SquareMatrixBloat::invert will be generated twice

  SquareMatrix<double, 5> sm1;
  sm1.invert();

  SquareMatrix<double, 10> sm2;
  sm2.invert();

  // business logic in SquareMatrixBase::invert will not be generated twice

  // the parameterized version may run slower: e.g. less opportunity for
  // compile-time optimization such as constant propagation.
  // but it also results in smaller binary size, which may better leverage
  // instruction cache locality.

  return 0;
}