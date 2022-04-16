# Assembly

[Godbolt](https://www.youtube.com/watch?v=bSkpMdDe4g4)
[Slides](https://github.com/CppCon/CppCon2017/blob/master/Keynotes/What%20Has%20My%20Compiler%20Done%20for%20Me%20Lately%20-%20Unbolting%20the%20Compiler%27s%20Lid/What%20Has%20My%20Compiler%20Done%20for%20Me%20Lately%20-%20Unbolting%20the%20Compiler%27s%20Lid%20-%20Matt%20Godbolt%20-%20CppCon%202017.pdf)

### Basics

##### Registers
rax (64b), eax (32b, higher 32b zeroed on write), ax (16b), ah (8b) al (8b)

* Register / ABI
rax - retval
rdi, rsi, rdx, rcx - params
rbp
rip
r8-r15
xmm0-xmm15

##### Intel syntax

```
op
op dest
op dest src1
op dest src1 src2
```

Examples
```
mov eax, dword ptr [r14]          ;  int eax = *14;
add rax, rdi                      ;  rax += rdi;
add eax, dword ptr [r14 + 4*rbx]  ;  eax += r14[rbx];
lea eax, [r14 + 4 * rbx]          ;  int *eax = &r14[rbx];
xor eax, eax                      ;  eax = 0;
```
Note double word is 16b.

Note the following common practices:
* Using `lea` for computation, instead of an `add` for the following, you may see
```
lea rax, [r14 + 4 * rbx]
```
Reason could have to do with you put the result directly into a designated place, maybe e.g. for retval
* Using `xor eax, eax` for ret 0: reason could have to do with shorter instruction than a `mov eax, 0`
* Avoiding expensive `div`
```
unsigned divide(unsigned x) { return x / 3; }

mov eax, edi         ;  eax = x
mov edx, 0xaaaaaaab  ;  edx = 0xaaaaaaab (2/3 shifted up by 2^32)
mul edx              ;  eax:edx = eax * edx (gets answer * 2^32)
mov eax, edx         ;  discards bottom 32 bits (gets answer * 2)
shr eax              ;  eax /= 2 (gets answer. why 2/3 and not 1/3? to cover the range of unsigned)
ret
```
And a modolus % 3 is above plus the following
```
lea eax, [rdx + rdx * 2]  ;  eax = result of division x3
sub edi, eax              ;  edi -= eax
mov eax, edi              ;  return eax
ret
```

How godbolt annotates c++ line / asm line correspondence: compiler generates this info with `-g` flag.

### Globals, linker

[talk](https://www.youtube.com/watch?v=xVT1y0xWgww)

Globals common uses: Logging, intrusive profiling, polymorphic factories

Code sample
```cpp
// static.h
extern std::string g_str;

// static.cc
std::string g_str = "something too long for SSO";

// dynamic.h
#include "static.h"

std::string& getGlobal();

// dynamic.cc
std::string& getGlobal() { return g_str; }

// main.cc
#include "dynamic.h"
#include "static.h"

int main() {
    std::cout << g_str << "\n";
    std::cout << getGlobal() << "\n";
}
```

If we link the above program in particular ways, it'll crash on double free: compile and archive the `static` library, compile and link statically against `static` to produce `dynamic`, and when building `main`, link against `static` first and then `dynamic`.

The printed `g_str` will live in main's address space (because `static` is link i.e. copied in first), its ctor will get called at entrace.
Because `dynamic` also statically linked in `static`, it'll also have a `g_str`, whose ctor will also get called when the shared library gets initialized at entrance, at the same address of main's `g_str` (linker thought them one and the same).
Then `atexit`, `static` and `dynamic` for `g_str` in main's address space will both happen and lead to a crash.

Since C++17, `inline std::string g_str` in the `.cc` solves it.

Ways to deal with undefined order of global instantiation in different TUs.

Consider destructor dependency: whereas appropriate, there might be a case for the likes of `folly::Indestructible` (uses a `union` with just template member `T` to avoid `~T` getting called).

What linker does:
- looks at link line left to right
- each target / library provides some symbols (`T`) and need others (`U`). `W` for weak symbols where multiple occurrences are Ok.
- pick up needed symbol at each encountered target / library
- each time a provided symbol matches one that's needed
  - if static, copy and paste assembly into target
  - if dynamic, say the loader will get this

[See also](tech-notes/linker/readme.md)

Different from traditional unix linker, clang linker (lld) keeps track of defined symbols even if they aren't requested by an object already seen, with this you don't have to specify the same library multiple times.

