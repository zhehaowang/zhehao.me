# Implementations

Coming up with appropriate definitions for your classes and declaration for your functions is often times lion's share of the battle, but there are still things in implementation to watch out for.

### Postpone variable definitions as long as possible

Unused variables come with the cost of ctors and dtors. Like this function
```cpp
// this function defines the variable "encrypted" too soon
std::string encryptPassword(const std::string& password)
{
  using namespace std;

  string encrypted;

  if (password.length() < MinimumPasswordLength) {
      throw logic_error("Password is too short");
  }
  ...                        // do whatever is necessary to place an
                             // encrypted version of password in encrypted
  return encrypted;
}
```
The object encrypted isn't completely unused, only if an exception is thrown. Why not move it after the length check instead?

With that the code is still not as tight as it might be: encrypted is defined without initialization.
Say you are going to want encrypted to hold the value of password initially, then given to an encrypt call
```cpp
void encrypt(std::string& s);             // encrypts s in place

// instead of
std::string encrypted;                // default-construct encrypted
encrypted = password;                 // assign to encrypted
encrypt(encrypted);

// just do
std::string encrypted(password);
encrypt(encrypted);
```

Not only should you postpone a variable's definition until right before you have to use the variable, you should also try to postpone the definition until you have initialization arguments for it.

What about loops?
```cpp
// Approach A: define outside loop   // Approach B: define inside loop

Widget w;
for (int i = 0; i < n; ++i){         for (int i = 0; i < n; ++i) {
  w = some value dependent on i;       Widget w(some value dependent on i);
  ...                                  ...
}                                    }

// 1 ctor + 1 dtor + n assignments   // n ctor + n dtor
```

A is generally more efficient if you know an assignment costs less than a ctor-dtor pair.
A also makes w visible in larger scope, something that's contrary to program comprehensibility and maintainability.

As a result, you should default to B unless you know assignment is less expensive than ctor-dtor pair and you are dealing with a performance sensitive part of your code.

**Takeaways**
* Postpone variable definitions as long as possible. It increases program clarity and improves program efficiency

### Minimize casting


