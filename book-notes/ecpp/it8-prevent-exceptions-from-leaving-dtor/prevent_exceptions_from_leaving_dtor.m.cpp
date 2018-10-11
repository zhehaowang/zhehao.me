#include <iostream>
#include <string>

// demonstrates an RAII class design where dtor may call an operation that throws
// but dtor handles it by swallowing. That is, only if client does not first call
// this operation that could throw in order to handle the exception.

class DBConn {
  public:
    DBConn() : d_handle(-1), d_isOpen(false) {}
    
    void open(int handle) {
      d_handle = handle;
      d_isOpen = true;
    }

    void close() {
      if (d_isOpen && d_handle > 10) {
        throw std::runtime_error("blow up");
      }
      d_isOpen = false;
    }

    ~DBConn() {
      if (d_isOpen) {
        try {
          close();
        } catch (...) {
          std::cout << "exception swallowed in dtor\n";
        }
      }
    }
  private:
    int  d_handle;
    bool d_isOpen;
};

int main() {
    DBConn conn;
    conn.open(11);
    const bool clientCares = true;

    if (clientCares) {
      // client is given the opportunity to handle this exception
      try {
        conn.close();
      } catch (const std::exception& e) {
        std::cout << "Client handles: " << e.what() << "\n";
      }
    } else {
      // or if client does not care, exception won't leave dtor
    }
    return 0;
}