#include <iostream>
#include <vector>
#include <fstream>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>

using namespace std;

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf32_Xword;
typedef int64_t Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;

/* Type for version symbol information.  */
typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;

#define EI_NIDENT (16)

// ELF header
typedef struct {
    unsigned char e_ident[EI_NIDENT];    /* Magic number and other info */
    Elf64_Half e_type;            /* Object file type */
    Elf64_Half e_machine;        /* Architecture */
    Elf64_Word e_version;        /* Object file version */
    Elf64_Addr e_entry;        /* Entry point virtual address */
    Elf64_Off e_phoff;        /* Program header table file offset */
    Elf64_Off e_shoff;        /* Section header table file offset */
    Elf64_Word e_flags;        /* Processor-specific flags */
    Elf64_Half e_ehsize;        /* ELF header size in OutputCodeDataSize */
    Elf64_Half e_phentsize;        /* Program header table entry size */
    Elf64_Half e_phnum;        /* Program header table entry count */
    Elf64_Half e_shentsize;        /* Section header table entry size */
    Elf64_Half e_shnum;        /* Section header table entry count */
    Elf64_Half e_shstrndx;        /* Section header string table index */
} Elf64_Ehdr;

// Program header
typedef struct {
    Elf64_Word p_type;     //4 OutputCodeDataSize
    Elf64_Word p_flags;    //4 OutputCodeDataSize. other fields - 8b long
    Elf64_Off p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    Elf64_Xword p_align;
} Elf64_Phdr;

// Section header
typedef struct {
    Elf64_Word sh_name;        /* Section name (string tbl index) */
    Elf64_Word sh_type;        /* Section type */
    Elf64_Xword sh_flags;        /* Section flags */
    Elf64_Addr sh_addr;        /* Section virtual addr at execution */
    Elf64_Off sh_offset;        /* Section file offset */
    Elf64_Xword sh_size;        /* Section size in OutputCodeDataSize */
    Elf64_Word sh_link;        /* Link to another section */
    Elf64_Word sh_info;        /* Additional section information */
    Elf64_Xword sh_addralign;        /* Section alignment */
    Elf64_Xword sh_entsize;        /* Entry size if section holds table */
} Elf64_Shdr;

#define PASCAL_STR_LEN_MAX 255

string NOP = "$90";

ofstream myfile;

int OutputCodeDataSize;
int startStubSize;
int endStubSize;

//shstrtab offset
Elf64_Off endingOffset;

int EndingStubSize;
Elf64_Off TextSectionOffs;
Elf64_Xword TextSectionSize;
Elf64_Off ShstrtabSectionOffs;
Elf64_Xword ShstrtabSectionSize;

Elf64_Off ElfHdrShoff_val_origin;
Elf64_Xword TextPhdrFilesz_val_origin;
Elf64_Xword TextSectionHdrSize_val_origin;
Elf64_Off ShstrtabSectionHdrOffs_val_origin;

vector<int> fileEntrails;
vector<int> endingEntrails;

Elf64_Off SymtabSectionOffs;

Elf64_Off StrtabSectionOffs;

Elf64_Ehdr elfHdr;
Elf64_Phdr dataHdr;
Elf64_Phdr textHdr;
Elf64_Shdr sectHdr;

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
#define LC_DYLD_ENVIRONMENT 0x27 /* string for dyld to treat
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
    uint32_t stroff;        /* string table offset */
    uint32_t strsize;    /* string table size in bytes */
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
vector<load_command> load_commands;

void findSectionHeaders(FILE *f) {

    rewind(f);
    fread(&MachHeader, 1, sizeof(MachHeader), f);
    cout << "MachOHeaderSize: " << hex << sizeof(MachHeader) << endl;

    cout << "magic: " << hex << MachHeader.magic << endl;
    cout << "filetype: " << hex << MachHeader.filetype << endl;
    cout << "ncmds: " << hex << MachHeader.ncmds << endl;
    cout << "sizeofcmds: " << hex << MachHeader.sizeofcmds << endl;

    cout << endl << endl;

    load_commands = vector<load_command>(MachHeader.ncmds);

    for (int i = 0; i < MachHeader.ncmds; ++i) {
        fread(&load_commands[i], 1, sizeof(load_commands[i]), f);

        cout << "Load commands: " << endl;
        cout << "cmd: " << hex << load_commands[i].cmd << endl;
        cout << "cmd_size: " << hex << load_commands[i].cmdsize << endl;

        if (load_commands[i].cmd == LC_SEGMENT_64) {
            segment_command_64 SC;
            fread(&SC, 1, sizeof(SC), f);
            cout << "Segment Command: " << endl;
            cout << "name: " << hex << SC.segname << endl;
            cout << "vmaddr: " << hex << SC.vmaddr << endl;
            cout << "vmsize: " << hex << SC.vmsize << endl;
            cout << "offset: " << hex << SC.fileoff << endl;
            cout << "size: " << hex << SC.filesize << endl;
            cout << "sections: " << hex << SC.nsects << endl;

            cout << endl << "Sections: " << endl;
            for (int j = 0; j < SC.nsects; ++j) {
                section_64 Section;
                fread(&Section, 1, sizeof(Section), f);
                cout << "section: " << Section.sectname << endl;
                cout << "addr: " << hex << Section.addr << endl;
                cout << "size: " << hex << Section.size << endl;
                cout << "offset: " << hex << Section.offset << endl;
                cout << "align: " << hex << Section.align << endl;
            }
        } else if (load_commands[i].cmd == LC_SYMTAB) {
            symtab_command SYMTAB;
            fread(&SYMTAB, 1, sizeof(SYMTAB), f);
            cout << "Symtab command:" << endl;
            cout << "symoff: " << hex << SYMTAB.symoff << endl;
            cout << "nsyms: " << hex << SYMTAB.nsyms << endl;
            cout << "stroff: " << hex << SYMTAB.stroff << endl;
            cout << "strsize: " << hex << SYMTAB.strsize << endl;
        } else if (load_commands[i].cmd == LC_UUID) {
            uuid_command UUID;
            fread(&UUID, 1, sizeof(UUID), f);
            cout << "UUID command:" << endl;
            cout << "uuid: " << hex << UUID.uuid << endl;
        } else if (load_commands[i].cmd == LC_SOURCE_VERSION) {
            source_version_command SVC;
            fread(&SVC, 1, sizeof(SVC), f);
            cout << "Source version command:" << endl;
            cout << "version: " << SVC.version << endl;
        } else if (load_commands[i].cmd == LC_UNIXTHREAD) {
            custom_thread_command SVC;
            fread(&SVC, 1, sizeof(SVC), f);
            cout << "Thread command:" << endl;
            cout << "flavor: " << SVC.flavor << endl;
            cout << "count: " << SVC.count << endl;
        }
        cout << endl << endl;
    }

    return;


    fread(&dataHdr, 1, sizeof(Elf64_Phdr), f);
    fread(&textHdr, 1, sizeof(Elf64_Phdr), f);

    //cout << "ElfHDRsize: " << hex << sizeof(Elf64_Ehdr) << "; PHDRsize: " << hex << sizeof(Elf64_Phdr) << endl;

    cout << "DataPhdrOffs:=$" << hex << dataHdr.p_offset << endl <<
         "DataPhdrSize:=$" << hex << dataHdr.p_filesz << endl;

    cout << "TextPhdrOffs:=$" << hex << textHdr.p_offset << endl <<
         "TextPhdrSize:=$" << hex << textHdr.p_filesz << endl;

    TextPhdrFilesz_val_origin = textHdr.p_filesz;

    fseek(f, elfHdr.e_shoff + elfHdr.e_shstrndx * sizeof(sectHdr), SEEK_SET);
    fread(&sectHdr, 1, sizeof(sectHdr), f);

    ElfHdrShoff_val_origin = elfHdr.e_shoff;

    cout << "sectHdrSize: " << hex << sizeof(Elf64_Shdr) << endl;
    cout << "e_shoff: " << hex << elfHdr.e_shoff << endl;

    cout << "textPhdrOffs_offs: " << hex << offsetof(Elf64_Phdr, p_filesz) << endl;
    cout << "PHDR sizeof: " << hex << sizeof(Elf64_Phdr) << endl;

    auto textPhdrOffsOffs = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + offsetof(Elf64_Phdr, p_filesz);
    //cout << "textphdr offs offs auto: " << hex << textPhdrOffsOffs << endl;

    auto sectHdrOffsOffs = elfHdr.e_shoff + 3 * sizeof(Elf64_Shdr) + offsetof(Elf64_Shdr, sh_offset);
    //cout << "sectHdrOffs_offs: " << hex << sectHdrOffsOffs << endl;

    auto memszOffs = offsetof(Elf64_Phdr, p_memsz);
    cout << "memszOffs: " << hex << memszOffs << endl;


    char *SectNames = (char *) malloc(sectHdr.sh_size);
    uint32_t idx;
    Elf64_Off sectionSize;
    Elf64_Off sectionOffset;

    fseek(f, sectHdr.sh_offset, SEEK_SET);
    fread(SectNames, 1, sectHdr.sh_size, f);

    cout << "# / name / offset(hex) / size(hex) / sh_name(hex)" << endl;

    for (idx = 0; idx < elfHdr.e_shnum; idx++) {

        const char *sectionName = "";

        fseek(f, elfHdr.e_shoff + idx * sizeof(sectHdr), SEEK_SET);
        fread(&sectHdr, 1, sizeof(sectHdr), f);

        if (sectHdr.sh_name)
            sectionName = SectNames + sectHdr.sh_name;
        sectionOffset = sectHdr.sh_offset;
        sectionSize = sectHdr.sh_size;
        auto section_sh_name = sectHdr.sh_name;

        cout << idx << " " <<
             sectionName << " " <<
             hex << sectionOffset << " " <<
             hex << sectionSize << " " <<
             hex << section_sh_name << endl;

        if (0 == strcmp(".text", sectionName)) {
            cout << "   TextSectionOffs:=$" << hex << sectionOffset << ";" << endl;
            cout << "   TextSectionSize:=$" << hex << sectionSize << ";" << endl;

            TextSectionOffs = sectionOffset;
            TextSectionSize = sectionSize;
        }

        if (0 == strcmp(".shstrtab", sectionName)) {
            cout << "   ShstrtabSectionOffs:=$" << hex << sectionOffset << ";" << endl;
            cout << "   ShstrtabSectionSize:=$" << hex << sectionSize << ";" << endl;

            endingOffset = sectionOffset;
            cout << "   endingOffset: " << hex << endingOffset << endl;

            ShstrtabSectionOffs = sectionOffset;
            ShstrtabSectionSize = sectionSize;

            ShstrtabSectionHdrOffs_val_origin = sectionOffset;
        }

        if (0 == strcmp(".symtab", sectionName)) {

            SymtabSectionOffs = sectionOffset;
        }
        if (0 == strcmp(".strtab", sectionName)) {

            StrtabSectionOffs = sectionOffset;
        }
    }
}

//fix code size and offs in program hdr and section
//fix shstrtab offset

int *copyEnding(FILE *f, Elf64_Off copyFrom) {

    rewind(f);
    fseek(f, copyFrom, SEEK_SET);

    while (!feof(f)) {

        endingEntrails.push_back(fgetc(f));
    }

    int size = (int) endingEntrails.size();
    EndingStubSize = size;


    int i = 0;
    string one("-1");
    string s = "  OutputCodeString(";
    int bytesInString = 0;
    vector<string> stringList;

    endStubSize = 0;
    while (i < size - 1) {

        if (PASCAL_STR_LEN_MAX == bytesInString) {
            s += ");";
            stringList.push_back(s);

            bytesInString = 0;
            s = "  OutputCodeString(";
        }

        int singleByte = endingEntrails[i];
        //if (0 == one.compare(to_string(singleByte))) cout << singleByte << endl;

        s += "#" + to_string(singleByte);

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
    //stringList.push_back("  EndingStubSize:=" + to_string(size) + ";");
    stringList.push_back("end;\n");

    //stringList.push_back("  e_shstrndxOffset:=" + to_string(0x3C) + ";");

    myfile << "\n{new}\nprocedure EmitEndingStub;\nbegin\n";
    for (i = 0; i < stringList.size()/*OutputCodeDataSize / PASCAL_STR_LEN_MAX*/; i++) {
        myfile << stringList[i] << endl;
    }


    return NULL;
}

void printConst() {

    myfile << "{new}\nconst EndingStubSize=$" << dec << EndingStubSize << ";\n";

    myfile << "\tStartStubSize=$" << hex << startStubSize << ";\n";
    myfile << "\tEndStubSize=$" << hex << endStubSize << ";\n";

    //myfile << "\tElfHdrShoff_offs=$28;\n";
    //TODO wtf tis value 4d4, not 4f0/530
    myfile << "\tElfHdrShoff_val0=$" << hex << ElfHdrShoff_val_origin << ";\n";


    //myfile << "\tTextPhdrFilesz_offs=$" << hex << (sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + 0x4) << ";\n";
    myfile << "\tTextPhdrFilesz_val0=$" << hex << TextPhdrFilesz_val_origin << ";\n";


    //myfile << "\tTxtSectHdrSize_offs=$" << hex << (ElfHdrShoff_val_origin + 2*sizeof(Elf64_Shdr) + 0x20) << ";\n";
    myfile << "\tTxtSectHdrSize_val0=$" << hex << TextSectionSize << ";\n";


    //myfile << "\tShSectHdrOffs_offs=$" << hex << (ElfHdrShoff_val_origin + 3*sizeof(Elf64_Shdr) + 0x18) << ";\n";
    myfile << "\tShSectHdrOffs_val0=$" << hex << ShstrtabSectionHdrOffs_val_origin << ";\n";

    myfile << "\tSymSHdrOffs_val0=$" << hex << SymtabSectionOffs << ";\n";
    myfile << "\tStrSHdrOffs_val0=$" << hex << StrtabSectionOffs << ";\n";


    myfile << "\tOffsElfHdrShoff=$" << hex << offsetof(Elf64_Ehdr, e_shoff) << ";\n";
    myfile << "\tOffsTextPHdrFilesz=$" << hex <<
           sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + offsetof(Elf64_Phdr, p_filesz) << ";\n";
    myfile << "\tOffsTextPHdrMemsz=$" << hex <<
           sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + offsetof(Elf64_Phdr, p_memsz) << ";\n";
    myfile << "\tOffsTextSectSize=$" << hex <<
           elfHdr.e_shoff + 2 * sizeof(Elf64_Shdr) + offsetof(Elf64_Shdr, sh_size) << ";\n";

    myfile << "\tOffsShstrSectOffs=$" << hex <<
           elfHdr.e_shoff + 3 * sizeof(Elf64_Shdr) + offsetof(Elf64_Shdr, sh_offset) << ";\n";
    myfile << "\tOffsSymtabSectOffs=$" << hex <<
           elfHdr.e_shoff + 4 * sizeof(Elf64_Shdr) + offsetof(Elf64_Shdr, sh_offset) << ";\n";
    myfile << "\tOffsStrtabSectOffs=$" << hex <<
           elfHdr.e_shoff + 5 * sizeof(Elf64_Shdr) + offsetof(Elf64_Shdr, sh_offset) << ";\n";

    /*cout << "\tTextSectionOffs=$" << hex << TextSectionOffs << ";\n";
    cout << "\tTextSectionSize=$" << hex << TextSectionSize << ";\n";
    cout << "\tShstrtabSectionOffs=$" << hex << ShstrtabSectionOffs << ";\n";
    cout << "\tShstrtabSectionSize=$" << hex << ShstrtabSectionSize << ";\n";*/
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

    myfile.open("/Users/a.mamaev/Work/CourseProject/A-Bauman-BTPC-macOS/rtl2pas/stub.txt");

    FILE *f = fopen(
            //"/Users/a.mamaev/Work/CourseProject/A-Bauman-BTPC-macOS/rtl64",
            "/Users/a.mamaev/Work/CourseProject/A-Bauman-BTPC-macOS/rtl64",
            "rwb");

    if (!f) {
        cout << "couldnt open file" << endl;
        return -1;
    }

    if (isExecutable(f)) {
        rewind(f);

        cout << "\nLooking up for section headers:" << endl;
        findSectionHeaders(f);


        //scan all
        rewind(f);
        while (feof(f)) {
            fileEntrails.push_back(fgetc(f));
        }
        cout << "OverallBytes:=" << fileEntrails.size() << endl;


        rewind(f);
        vector<string> stringList;
        stringList.clear();

        stringList.push_back("{ab}\nprocedure EmitStubCode;\nbegin");
        stringList.push_back("  OutputCodeDataSize:=0;");

        string s = "  OutputCodeString(";
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
            s += "#" + to_string(singleByte);

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
        stringList.push_back("  OutputCodeDataSize:=" + to_string(OutputCodeDataSize) + ";");
        stringList.push_back("end;");

        for (int i = 0; i < stringList.size()/*OutputCodeDataSize / PASCAL_STR_LEN_MAX + 1 + 2*/; i++) {

            myfile << stringList[i] << endl;
        }


        //EmitEndingCode

        int *ending = copyEnding(f, endingOffset);


        printConst();

    } else {
        cout << "file is not of ELF format" << endl;
    }

    myfile.close();
    return 0;
}
