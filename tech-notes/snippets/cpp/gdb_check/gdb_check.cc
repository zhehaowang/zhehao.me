#include <string>

int func(int x, int y) __attribute__((noinline));
int func(int x, int y) {
    return x + y;
}

int main() {
    func(2, 3);
    return 0;
}

// We compile with -O0 and -fno-omit-frame-pointer.
// When compiling without -g, gdb also manages to see function names.
// 
// (gdb) info functions
// Non-debugging symbols:
// 0x0000000000001000  _init
// 0x0000000000001030  __cxa_finalize@plt
// 0x0000000000001040  _start
// 0x0000000000001070  deregister_tm_clones
// 0x00000000000010a0  register_tm_clones
// 0x00000000000010e0  __do_global_dtors_aux
// 0x0000000000001120  frame_dummy
// 0x0000000000001125  func(int, int)
// 0x0000000000001139  main
// 0x0000000000001160  __libc_csu_init
// 0x00000000000011c0  __libc_csu_fini
// 0x00000000000011c4  _fini
// 
// ELF sections:
// $ readelf -S main
// 
// The functions gdb found corresponds with the output of this
// $ readelf -a main | rg FUNC
// 
// The address (where the code is mapped in virtual address space) of readelf and gdb agree, too.
// 
// To look at main
// (gdb) disas 0x0000000000001139
// or
// (gdb) disas main
//    0x0000000000001139 <+0>:     push   %rbp               ; save rbp
//    0x000000000000113a <+1>:     mov    %rsp,%rbp          ; update rbp
//    0x000000000000113d <+4>:     mov    $0x3,%esi          ; prepare arg1
//    0x0000000000001142 <+9>:     mov    $0x2,%edi          ; prepare arg2
//    0x0000000000001147 <+14>:    callq  0x1125 <_Z4funcii> ; call func(x,y)
//    0x000000000000114c <+19>:    mov    $0x0,%eax          ; prepare retval
//    0x0000000000001151 <+24>:    pop    %rbp               ; remove rbp
//    0x0000000000001152 <+25>:    retq                      ; done
// 
// (gdb) b main
// (gdb) r
// (gdb) disas
//    0x0000555555555139 <+0>:     push   %rbp
//    0x000055555555513a <+1>:     mov    %rsp,%rbp
// => 0x000055555555513d <+4>:     mov    $0x3,%esi          ; we broke after rbp adjustments were made
//    0x0000555555555142 <+9>:     mov    $0x2,%edi
//    0x0000555555555147 <+14>:    callq  0x555555555125 <_Z4funcii>
//    0x000055555555514c <+19>:    mov    $0x0,%eax
//    0x0000555555555151 <+24>:    pop    %rbp
//    0x0000555555555152 <+25>:    retq
// (gdb) b func
// (gdb) c
// (gdb) disas
//    0x0000555555555125 <+0>:     push   %rbp
//    0x0000555555555126 <+1>:     mov    %rsp,%rbp
// => 0x0000555555555129 <+4>:     mov    %edi,-0x4(%rbp)
//    0x000055555555512c <+7>:     mov    %esi,-0x8(%rbp)
//    0x000055555555512f <+10>:    mov    -0x4(%rbp),%edx
//    0x0000555555555132 <+13>:    mov    -0x8(%rbp),%eax
//    0x0000555555555135 <+16>:    add    %edx,%eax
//    0x0000555555555137 <+18>:    pop    %rbp
//    0x0000555555555138 <+19>:    retq
// (gdb) bt
// #0  0x0000555555555129 in func(int, int) ()
// #1  0x000055555555514c in main ()
// (gdb) i f 0
// Stack level 0, frame at 0x7fffffffd500:
//  rip = 0x555555555129 in func(int, int); saved rip = 0x55555555514c
//  called by frame at 0x7fffffffd510
//  Arglist at 0x7fffffffd4f0, args:
//  Locals at 0x7fffffffd4f0, Previous frame's sp is 0x7fffffffd500
//  Saved registers:
//   rbp at 0x7fffffffd4f0, rip at 0x7fffffffd4f8
// 
// Let's look at this stack frame
// (gdb) i r rsp
// rsp            0x7fffffffd4f0      0x7fffffffd4f0
// (gdb) i r rbp
// rbp            0x7fffffffd4f0      0x7fffffffd4f0
// 
// rbp and rsp are the same, and the content of rbp should be saved rbp
// i.e. last stack frame's rbp (the memory address of the start of the
// last frame)
// and rbp+8 should be return address (saved rip)
// 
// (gdb) x/x $rbp
// 0x7fffffffd4f0: 0xffffd500
// (gdb) x/x $rbp+8
// 0x7fffffffd4f8: 0x5555514c
// 
// (gdb) i f 1
// Stack frame at 0x7fffffffd510:
//  rip = 0x55555555514c in main; saved rip = 0x7ffff7a6609b
//  caller of frame at 0x7fffffffd500
//  Arglist at 0x7fffffffd500, args:
//  Locals at 0x7fffffffd500, Previous frame's sp is 0x7fffffffd510
//  Saved registers:
//   rbp at 0x7fffffffd500, rip at 0x7fffffffd508
// 
// So, the stack at this point looks like
// (gdb) x/100gx 0x7fffffffd4f0
// 0x7fffffffd4f0:                    
//                 0x00007fffffffd500 <== registers rbp, rsp
//            4f8:                    
//                 0x000055555555514c <== f0 return addr (saved rip)
// 0x7fffffffd500:                    <== start of frame 0 (func)
//                 0x0000555555555160 <== f1 saved rbp
//            508: 
//                 0x00007ffff7a6609b <== f1 return addr
// 0x7fffffffd510:                    <== start of frame 1 (main)
//                 0xffffffffffffff90   
// 
// so rbp forms a linked list to traverse the stack
//
// See also performance-engineering.md for an illustration of a stack frame
// 