#include <iostream>
#include <string>
#include <functional>

// demonstrates alternatives to public virtual methods, in particular, passing
// a std::function (for flexibility in what can be given) in ctor to allow
// per-object customization, but restrict yourself to using only public
// information in the function object.

class GameCharacter {
public:
  // HealthCalcFunc is any callable entity that can be called with
  // anything compatible with a GameCharacter and that returns anything
  // compatible with an int; see below for details
  ~GameCharacter() = default;

  using HealthCalcFunc = std::function<int (const GameCharacter&)>;

  explicit GameCharacter(HealthCalcFunc hcf)
   : healthFunc(hcf) {}

  int healthValue() const {
    return healthFunc(*this);
  }
private:
  HealthCalcFunc healthFunc;
};

// like a strategy pattern, but instead of virtual healthCalc inside
// GameCharacter object, we pass the function in.

// it can be a free function (note the non-int return type's implicit
// conversion) 
short calcHealth(const GameCharacter&) {
  std::cout << "calcHealth() (free function)\n";
  return 0;
}

// it can be a function object
struct HealthCalculator {
  int operator() (const GameCharacter&) {
    std::cout << "HealthCalculator::operator() (function object)\n";
    return 1;
  }
};

// it can be a member function
struct GameLevel {
  float health(const GameCharacter&) const {
    std::cout << "GameLevel::health() (member function)\n";
    return 2.0;
  }
};

class EvilBadGuy : public GameCharacter {
public:
  EvilBadGuy(GameCharacter::HealthCalcFunc hf) : GameCharacter(hf) {}
};

class EyeCandyCharacter : public GameCharacter {
public:
  EyeCandyCharacter(GameCharacter::HealthCalcFunc hf) : GameCharacter(hf) {}
};

int main() {
  EvilBadGuy ebg1(calcHealth);                      // character using a
                                                    // health calculation
                                                    // function
  HealthCalculator hcObj;
  EyeCandyCharacter ecc1(hcObj);                    // character using a
                                                    // health calculation
                                                    // function object

  GameLevel currentLevel;

  EvilBadGuy ebg2(                                  // character using a
    std::bind(&GameLevel::health,                   // health calculation
              currentLevel,                         // member function;
              std::placeholders::_1)                // see below for details
  );

  ebg1.healthValue();
  ecc1.healthValue();
  ebg2.healthValue();
  return 0;
}