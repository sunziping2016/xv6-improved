// Format of an ELF executable file

#define ELF_MAGIC 0x464C457FU  // "\x7FELF" in little endian

// File header
struct elfhdr {
    uint magic;  // must equal ELF_MAGIC
    uchar elf[12];
    ushort type;
    ushort machine;
    uint version;
    uint entry;
    uint phoff; // program header table offset
    uint shoff; // section header table offset
    uint flags;
    ushort ehsize;
    ushort phentsize; // size of program header table entry
    ushort phnum; // number of entries in program header table
    ushort shentsize; // size of ection header table entry
    ushort shnum; // number of entries in section header table
    ushort shstrndx; // index of section header table entry that contains section names
};

// Program section header
struct proghdr {
    uint type;
    uint off;
    uint vaddr;
    uint paddr;
    uint filesz;
    uint memsz;
    uint flags;
    uint align;
};

// Values for Proghdr type
#define ELF_PROG_LOAD           1

// Flag bits for Proghdr flags
#define ELF_PROG_FLAG_EXEC      1
#define ELF_PROG_FLAG_WRITE     2
#define ELF_PROG_FLAG_READ      4

//PAGEBREAK!
// Blank page.
