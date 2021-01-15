# A Bauman BeRo TinyPascal

A ported version of self-hosting capable [BeRo Tiny Pascal Compiler](https://github.com/BeRo1985/berotinypascal) for the macOS 64-bit platform

## How-to-use

### Basic compiler usage

```bash
btpc64 < myProgram.pas > myProgram
```

### Runtime reassemble

```bash
gcc -c rtl64.s
ld rtl64.o -g -o rtl64 -T linkerScript.ld -nostdlib
```
     
## Porting notes

macOS 64-bit porting author - [Alexey Mamaev](https://github.com/AleksMa).

Linux 64-bit porting author - [Anthony Belyaev](https://github.com/avbelyaev).

Original BTPC author - [Benjamin Rosseaux](https://github.com/BeRo1985).
