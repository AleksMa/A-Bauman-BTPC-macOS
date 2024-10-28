#include <cstring>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

#define EI_NIDENT (16)
#define PASCAL_STR_LEN_MAX 20

const std::string NOP = "$90";

std::ofstream myfile;

int OutputCodeDataSize;
int startStubSize;
int endStubSize;

uint32_t endingOffset = 0x1000;

int EndingStubSize;

std::vector<int> fileEntrails;
std::vector<int> endingEntrails;

#include <mach/machine.h>
#include <mach/vm_prot.h>
#include <mach/machine/thread_status.h>
#include <architecture/byte_order.h>

struct mach_header {
    uint32_t magic;          // mach magic number
    cpu_type_t cputype;        // cpu specifier
    cpu_subtype_t cpusubtype;     // cpu subtype specifier
    uint32_t filetype;       // type of mach-o e.g. exec, dylib ...
    uint32_t ncmds;          // number of load commands
    uint32_t sizeofcmds;     // size of load command region
    uint32_t flags;          // flags
    uint32_t reserved;       // *64-bit only* reserved
};

struct load_command {
    uint32_t cmd;            // type of load command
    uint32_t cmdsize;        // size of load command
};

#define    LC_SEGMENT    0x1    /* segment of this file to be mapped */
#define    LC_SYMTAB    0x2    /* link-edit stab symbol table info */
#define    LC_SYMSEG    0x3    /* link-edit gdb symbol table info (obsolete) */
#define    LC_THREAD    0x4    /* thread */
#define    LC_UNIXTHREAD    0x5    /* unix thread (includes a stack) */
#define    LC_LOADFVMLIB    0x6    /* load a specified fixed VM shared library */
#define    LC_IDFVMLIB    0x7    /* fixed VM shared library identification */
#define    LC_IDENT    0x8    /* object identification info (obsolete) */
#define LC_FVMFILE    0x9    /* fixed VM file inclusion (internal use) */
#define LC_PREPAGE      0xa     /* prepage command (internal use) */
#define    LC_DYSYMTAB    0xb    /* dynamic link-edit symbol table info */
#define    LC_LOAD_DYLIB    0xc    /* load a dynamically linked shared library */
#define    LC_ID_DYLIB    0xd    /* dynamically linked shared lib ident */
#define LC_LOAD_DYLINKER 0xe    /* load a dynamic linker */
#define LC_ID_DYLINKER    0xf    /* dynamic linker identification */
#define    LC_PREBOUND_DYLIB 0x10    /* modules prebound for a dynamically */
/*  linked shared library */
#define    LC_ROUTINES    0x11    /* image routines */
#define    LC_SUB_FRAMEWORK 0x12    /* sub framework */
#define    LC_SUB_UMBRELLA 0x13    /* sub umbrella */
#define    LC_SUB_CLIENT    0x14    /* sub client */
#define    LC_SUB_LIBRARY  0x15    /* sub library */
#define    LC_TWOLEVEL_HINTS 0x16    /* two-level namespace lookup hints */
#define    LC_PREBIND_CKSUM  0x17    /* prebind checksum */

/*
 * load a dynamically linked shared library that is allowed to be missing
 * (all symbols are weak imported).
 */
#define    LC_LOAD_WEAK_DYLIB (0x18 | LC_REQ_DYLD)

#define    LC_SEGMENT_64    0x19    /* 64-bit segment of this file to be
				   mapped */
#define    LC_ROUTINES_64    0x1a    /* 64-bit image routines */
#define LC_UUID        0x1b    /* the uuid */
#define LC_RPATH       (0x1c | LC_REQ_DYLD)    /* runpath additions */
#define LC_CODE_SIGNATURE 0x1d    /* local of code signature */
#define LC_SEGMENT_SPLIT_INFO 0x1e /* local of info to split segments */
#define LC_REEXPORT_DYLIB (0x1f | LC_REQ_DYLD) /* load and re-export dylib */
#define    LC_LAZY_LOAD_DYLIB 0x20    /* delay load of dylib until first use */
#define    LC_ENCRYPTION_INFO 0x21    /* encrypted segment information */
#define    LC_DYLD_INFO    0x22    /* compressed dyld information */
#define    LC_DYLD_INFO_ONLY (0x22|LC_REQ_DYLD)    /* compressed dyld information only */
#define    LC_LOAD_UPWARD_DYLIB (0x23 | LC_REQ_DYLD) /* load upward dylib */
#define LC_VERSION_MIN_MACOSX 0x24   /* build for MacOSX min OS version */
#define LC_VERSION_MIN_IPHONEOS 0x25 /* build for iPhoneOS min OS version */
#define LC_FUNCTION_STARTS 0x26 /* compressed table of function start addresses */
#define LC_DYLD_ENVIRONMENT 0x27 /* std::string for dyld to treat
				    like environment variable */
#define LC_MAIN (0x28|LC_REQ_DYLD) /* replacement for LC_UNIXTHREAD */
#define LC_DATA_IN_CODE 0x29 /* table of non-instructions in __text */
#define LC_SOURCE_VERSION 0x2A /* source version used to build binary */
#define LC_DYLIB_CODE_SIGN_DRS 0x2B /* Code signing DRs copied from linked dylibs */

struct segment_command_64 { /* for 64-bit architectures */
    // uint32_t	cmd;		/* LC_SEGMENT_64 */
    // uint32_t	cmdsize;	/* includes sizeof section_64 structs */
    char segname[16];    /* segment name */
    uint64_t vmaddr;        /* memory address of this segment */
    uint64_t vmsize;        /* memory size of this segment */
    uint64_t fileoff;    /* file offset of this segment */
    uint64_t filesize;    /* amount to map from the file */
    vm_prot_t maxprot;    /* maximum VM protection */
    vm_prot_t initprot;    /* initial VM protection */
    uint32_t nsects;        /* number of sections in segment */
    uint32_t flags;        /* flags */
};

struct section_64 {
    char sectname[16];   /* name of this section */
    char segname[16];    /* segment this section goes in */
    uint64_t addr;           /* memory address of this section */
    uint64_t size;           /* size in bytes of this section */
    uint32_t offset;         /* file offset of this section */
    uint32_t align;          /* section alignment (power of 2) */
    uint32_t reloff;         /* file offset of relocation entries */
    uint32_t nreloc;         /* number of relocation entries */
    uint32_t flags;          /* flags (section type and attributes)*/
    uint32_t reserved1;      /* reserved (for offset or index) */
    uint32_t reserved2;      /* reserved (for count or sizeof) */
    uint32_t reserved3;      /* reserved */
};

struct symtab_command {
    uint32_t symoff;        /* symbol table offset */
    uint32_t nsyms;        /* number of symbol table entries */
    uint32_t stroff;        /* std::string table offset */
    uint32_t strsize;    /* std::string table size in bytes */
};

struct uuid_command {
    uint8_t uuid[16];    /* the 128-bit uuid */
};

struct source_version_command {
    uint64_t version;    /* A.B.C.D.E packed as a24.b10.c10.d10.e10 */
};

struct custom_thread_command {
    uint32_t flavor;
    uint32_t count;
    x86_thread_state64_t state;
};

mach_header MachHeader;
std::vector<load_command> load_commands;

void findSectionHeaders(FILE *f) {

    rewind(f);
    fread(&MachHeader, 1, sizeof(MachHeader), f);
    std::cout << "MachOHeaderSize: " << std::hex << sizeof(MachHeader) << std::endl;

    std::cout << "magic: " << std::hex << MachHeader.magic << std::endl;
    std::cout << "filetype: " << std::hex << MachHeader.filetype << std::endl;
    std::cout << "ncmds: " << std::hex << MachHeader.ncmds << std::endl;
    std::cout << "sizeofcmds: " << std::hex << MachHeader.sizeofcmds << std::endl;

    std::cout << std::endl << std::endl;

    load_commands = std::vector<load_command>(MachHeader.ncmds);

    for (int i = 0; i < MachHeader.ncmds; ++i) {
        std::fread(&load_commands[i], 1, sizeof(load_commands[i]), f);

        std::cout << "Load commands: " << std::endl;
        std::cout << "cmd: " << std::hex << load_commands[i].cmd << std::endl;
        std::cout << "cmd_size: " << std::hex << load_commands[i].cmdsize << std::endl;

        if (load_commands[i].cmd == LC_SEGMENT_64) {
            segment_command_64 SC;
            fread(&SC, 1, sizeof(SC), f);
            std::cout << "Segment Command: " << std::endl;
            std::cout << "name: " << std::hex << SC.segname << std::endl;
            std::cout << "vmaddr: " << std::hex << SC.vmaddr << std::endl;
            std::cout << "vmsize: " << std::hex << SC.vmsize << std::endl;
            std::cout << "offset: " << std::hex << SC.fileoff << std::endl;
            std::cout << "size: " << std::hex << SC.filesize << std::endl;
            std::cout << "sections: " << std::hex << SC.nsects << std::endl;

            std::cout << std::endl << "Sections: " << std::endl;
            for (int j = 0; j < SC.nsects; ++j) {
                section_64 Section;
                fread(&Section, 1, sizeof(Section), f);
                std::cout << "section: " << Section.sectname << std::endl;
                std::cout << "addr: " << std::hex << Section.addr << std::endl;
                std::cout << "size: " << std::hex << Section.size << std::endl;
                std::cout << "offset: " << std::hex << Section.offset << std::endl;
                std::cout << "align: " << std::hex << Section.align << std::endl;
            }
        } else if (load_commands[i].cmd == LC_SYMTAB) {
            symtab_command SYMTAB;
            fread(&SYMTAB, 1, sizeof(SYMTAB), f);
            std::cout << "Symtab command:" << std::endl;
            std::cout << "symoff: " << std::hex << SYMTAB.symoff << std::endl;
            std::cout << "nsyms: " << std::hex << SYMTAB.nsyms << std::endl;
            std::cout << "stroff: " << std::hex << SYMTAB.stroff << std::endl;
            std::cout << "strsize: " << std::hex << SYMTAB.strsize << std::endl;
        } else if (load_commands[i].cmd == LC_UUID) {
            uuid_command UUID;
            fread(&UUID, 1, sizeof(UUID), f);
            std::cout << "UUID command:" << std::endl;
            std::cout << "uuid: " << std::hex << UUID.uuid << std::endl;
        } else if (load_commands[i].cmd == LC_SOURCE_VERSION) {
            source_version_command SVC;
            fread(&SVC, 1, sizeof(SVC), f);
            std::cout << "Source version command:" << std::endl;
            std::cout << "version: " << SVC.version << std::endl;
        } else if (load_commands[i].cmd == LC_UNIXTHREAD) {
            custom_thread_command SVC;
            fread(&SVC, 1, sizeof(SVC), f);
            std::cout << "Thread command:" << std::endl;
            std::cout << "flavor: " << SVC.flavor << std::endl;
            std::cout << "count: " << SVC.count << std::endl;
        }
        std::cout << std::endl << std::endl;
    }

    return;
}

//fix code size and offs in program hdr and section
//fix shstrtab offset

int *copyEnding(FILE *f, uint32_t copyFrom) {

    rewind(f);
    fseek(f, copyFrom, SEEK_SET);

    while (!feof(f)) {
        endingEntrails.push_back(fgetc(f));
    }

    int size = (int) endingEntrails.size();
    EndingStubSize = size;


    int i = 0;
    std::string one("-1");
    std::string s = "  OutputCodeString(";
    int bytesInString = 0;
    std::vector<std::string> stringList;

    endStubSize = 0;
    while (i < size - 1) {

        if (PASCAL_STR_LEN_MAX == bytesInString) {
            s += ");";
            stringList.push_back(s);

            bytesInString = 0;
            s = "  OutputCodeString(";
        }

        int singleByte = endingEntrails[i];

        s += "#" + std::to_string(singleByte);

        bytesInString++;
        i++;
        endStubSize++;
    }

    while (bytesInString < PASCAL_STR_LEN_MAX) {

        s += "#0";// + NOP;
        bytesInString++;
        endStubSize++;
    }
    s += ");";

    stringList.push_back(s);
    stringList.push_back("end;\n");

    myfile << "\nprocedure EmitEndingStub;\nbegin\n";
    for (i = 0; i < stringList.size(); i++) {
        myfile << stringList[i] << std::endl;
    }

    return NULL;
}

void printConst() {

    myfile << "const EndingStubSize=" << std::dec << EndingStubSize << ";\n";

    myfile << "\tStartStubSize=$" << std::hex << startStubSize << ";\n";
    myfile << "\tEndStubSize=$" << std::hex << endStubSize << ";\n";
}

bool isExecutable(FILE *f) {

    fgetc(f);
    if ('E' == fgetc(f) &&
        'L' == fgetc(f) &&
        'F' == fgetc(f)) {
        return true;
    }

    rewind(f);
    return 0xcf == fgetc(f) &&
           0xfa == fgetc(f) &&
           0xed == fgetc(f) &&
           0xfe == fgetc(f);
}

int main() {

    myfile.open("stub.txt");

    FILE *f = fopen("rtl64macOS", "rwb");

    if (!f) {
        std::cout << "could not open file" << std::endl;
        return -1;
    }

    if (isExecutable(f)) {
        rewind(f);

        std::cout << "\nLooking up for section headers:" << std::endl;
        findSectionHeaders(f);

        rewind(f);
        while (!feof(f)) {
            fileEntrails.push_back(fgetc(f));
        }
        std::cout << "OverallBytes:=" << fileEntrails.size() << std::endl;

        rewind(f);
        std::vector<std::string> stringList;
        stringList.clear();

        stringList.push_back("procedure EmitStubCode;\nbegin");
        stringList.push_back("  OutputCodeDataSize:=0;");

        std::string s = "  OutputCodeString(";
        int singleByte;

        int bytesInString = 0;
        OutputCodeDataSize = 0;

        while (OutputCodeDataSize < (int) endingOffset) {

            if (PASCAL_STR_LEN_MAX == bytesInString) {
                s += ");";
                stringList.push_back(s);

                bytesInString = 0;
                s = "  OutputCodeString(";
            }

            singleByte = fgetc(f);
            s += "#" + std::to_string(singleByte);

            bytesInString++;
            OutputCodeDataSize++;
            startStubSize++;
        }

        while (bytesInString < PASCAL_STR_LEN_MAX) {

            s += "#" + NOP;
            bytesInString++;
            startStubSize++;
        }
        s += ");";

        stringList.push_back(s);
        stringList.push_back("  OutputCodeDataSize:=" + std::to_string(OutputCodeDataSize) + ";");
        stringList.push_back("end;");

        for (int i = 0; i < stringList.size()/*OutputCodeDataSize / PASCAL_STR_LEN_MAX + 1 + 2*/; i++) {

            myfile << stringList[i] << std::endl;
        }

        int *ending = copyEnding(f, endingOffset);

        printConst();
    } else {
        std::cout << "file is not of ELF format" << std::endl;
    }

    myfile.close();
    return 0;
}
