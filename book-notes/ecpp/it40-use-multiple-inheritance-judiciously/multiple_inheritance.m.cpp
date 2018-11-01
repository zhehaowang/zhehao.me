#include <iostream>
#include <string>
#include <memory>

// demonstrates a case where arguably multiple inheritance is useful: inherit
// an interface from a class, and implementation from another (because we want
// to override its virtual functions)

class IPerson {                            // this class specifies the
public:                                    // interface to be implemented
  virtual ~IPerson() = default;

  virtual std::string name() const = 0;
}; 

class PersonInfo {                         // this class has functions
public:                                    // useful in implementing
                                           // the IPerson interface
  virtual ~PersonInfo() = default;

  virtual const char * theName() const {
    std::cout << valueDelimOpen() << "theName called"
              << valueDelimClose() << "\n";
    return "theName";
  }

  virtual const char * valueDelimOpen() const = 0;;
    // made abstract just for the sake
    // of demonstration

  virtual const char * valueDelimClose() const = 0;
};

class CPerson: public IPerson, private PersonInfo {
  // note use of MI: inherit interface from one class, implementation from
  // another (due to wanting to override some of the latter's virtual functions)
public:
  virtual std::string name() const                      // implementations
  { return PersonInfo::theName(); }                     // of the required
                                                        // IPerson member
                                                        // functions
private:                                                // redefinitions of
  const char * valueDelimOpen() const { return "["; }    // inherited virtual
  const char * valueDelimClose() const { return "]"; }   // delimiter
};                                                       // functions

int main() {
  std::unique_ptr<IPerson> pp = std::make_unique<CPerson>();
  pp->name();
  return 0;
}