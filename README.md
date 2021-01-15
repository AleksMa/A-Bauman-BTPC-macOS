# A Bauman BeRo TinyPascal macOS

A ported version of self-hosting capable [BeRo Tiny Pascal Compiler](https://github.com/BeRo1985/berotinypascal) for the macOS 64-bit platform

## How-to-use

### Basic compiler usage

```bash
btpc64macOS < myProgram.pas > myProgram
```

### Bootstapping

```bash
btpc64macOS < btpc64macOS.pas > btpc64macOSCheck
diff btpc64macOS btpc64macOSCheck
```

### Runtime reassemble

```bash
as rtl64.s -o rtl64.o
ld rtl64.o -e _main -o rtl64 -lSystem
```
     
## Porting notes

macOS 64-bit porting author - [Alexey Mamaev](https://github.com/AleksMa).

Linux 64-bit porting author - [Anthony Belyaev](https://github.com/avbelyaev).

Original BTPC author - [Benjamin Rosseaux](https://github.com/BeRo1985).
