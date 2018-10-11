#include <iostream>
#include <string>
#include <sstream>

// demonstrates a way to avoid calling virtual functions in ctor

class Transaction {
public:
  Transaction(const std::string& msg) {
    // you may be tempted to make logTransaction virtual and overridden
    // in both children below, to print their corresponding messages!
    // Don't! As if you do, here it'll always call the base class's
    // logTransaction!
    logTransaction(msg);
  }

  virtual ~Transaction() = 0;
private:
  void logTransaction(const std::string& msg) {
    std::cout << msg << "\n";
  }
};

Transaction::~Transaction() {}

class BuyTransaction : public Transaction {
public:
  BuyTransaction() : Transaction(getTypeMessage()) {}
private:
  static std::string getTypeMessage() {
    return "one buy transaction";
  }
  int d_size;
};

class SellTransaction : public Transaction {
public:
  SellTransaction(int size) : Transaction(getTypeMessage(size)) {}
private:
  static std::string getTypeMessage(int size) {
     std::stringstream ss;
     ss << "one sell transaction of size " << size;
     return ss.str();
  }
  int d_size;
};

int main() {
  BuyTransaction  bt;
  SellTransaction st(20);
  return 0;
}