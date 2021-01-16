# A Bauman BeRo TinyPascal macOS

A ported version of self-hosting capable [BeRo Tiny Pascal Compiler](https://github.com/BeRo1985/berotinypascal) for the macOS 64-bit platform

## Details

* `rtl64macOS.s` - RTL asm source file for macOS
* `btpc64macOS.pas` - compiler source code on Pascal
* `btpc64macOS` - Mach-O compiler executable
* `/bootstrapping/btpc64macOSCrossLinux` - ELF crosscompiler executable (Pascal -> Mach-O)
* `/bootstrapping/tests/` - simple programs for testing compiler and crosscompiler
* `/rtl2pas/` - RTL Mach-O executable & parsing cpp script

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

### RTL reassemble

```bash
as rtl64.s -o rtl64.o
ld rtl64.o -e _main -o rtl64 -lSystem
```
     
## Porting notes

macOS 64-bit porting author - [Alexey Mamaev](https://github.com/AleksMa).

[Linux 64-bit](https://github.com/bmstu-iu9/A-Bauman-BTPC-64) porting author - [Anthony Belyaev](https://github.com/avbelyaev).

Original BTPC author - [Benjamin Rosseaux](https://github.com/BeRo1985).
