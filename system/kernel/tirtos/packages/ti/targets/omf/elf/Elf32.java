package ti.targets.omf.elf;

import java.io.RandomAccessFile;
import java.io.IOException;
import java.util.HashMap;
import java.util.ArrayList;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.io.BufferedInputStream;
import java.io.FileInputStream;

import java.util.zip.CRC32;

import java.util.regex.Pattern;
import java.util.regex.Matcher;

/*
 *  ======== Elf32 ========
 */
public class Elf32 implements xdc.rta.IOFReader, xdc.rov.ISymbolTable
{
    /*
     *  ======== C_IDENT ========
     *  Regular expression for C-identifies optionally followed by
     *  TI's  program-level optimization mangling hack; i.e., the addition
     *  of "$<file number>" to each static symbol in a file participating in
     *  program-level optimization.
     */
    static final Pattern C_IDENT =
        Pattern.compile("[a-zA-Z_][a-zA-Z0-9_]*(\\$[0-9]+)?");
    static Matcher cIdent = C_IDENT.matcher("");
    
    static final int ET_NONE = 0;         /* No machine */
    static final int EM_M32 = 1;          /* AT&T WE 32100 */
    static final int EM_SPARC = 2;        /* SPARC */
    static final int EM_386 = 3;          /* Intel Architecture */
    static final int EM_68K = 4;          /* Motorola 68000 */
    static final int EM_88K = 5;          /* Motorola 88000 */
    static final int EM_860 = 7;          /* Intel 80860 */
    static final int EM_MIPS = 8;         /* MIPS RS3000 Big-Endian */
    static final int EM_MIPS_RS4_BE = 10; /* MIPS RS4000 Big-Endian */
    static final int EM_ARM = 40;         /* Advanced RISC Machines ARM */
    static final int EM_MSP430 =   105;   /* TI msp430*/
    static final int EM_TI_C6000 = 140;   /* TI TMS320C6000 DSP family */
    static final int EM_TI_C2000 = 141;   /* TI TMS320C2000 DSP family */
    static final int EM_TI_C5500 = 142;   /* TI TMS320C55x DSP family */
    static final int EM_TI_T16   = 143;   /* TI T16 family */

    static final int EI_MAG0 = 0;         /* File identification */
    static final int EI_MAG1 = 1;         /* File identification */
    static final int EI_MAG2 = 2;         /* File identification */
    static final int EI_MAG3 = 3;         /* File identification */
    static final int EI_CLASS = 4;        /* File class */
    static final int EI_DATA = 5;         /* Data encoding */
    static final int EI_VERSION = 6;      /* File version */
    static final int EI_PAD = 7;          /* Start of padding bytes */
    static final int EI_NIDENT = 16;      /* Size of e_ident[] */

    static final int ELFDATANONE = 0;  /* Invalid data encoding */
    static final int ELFDATA2LSB = 1;  /* Little-endian data    */
    static final int ELFDATA2MSB = 2;  /* Big-endian data       */

    /*
     *  File Header flags
     */
    static final int F_VERS0 = 0x0;
    static final int F_VERS1 = 0x10;
    static final int F_VERS2 = 0x20;
    static final int F_VERS3 = 0x40;
    static final int F_VERS4 = 0x80;
    static final int F_VERSION = (F_VERS1 | F_VERS2 | F_VERS3 | F_VERS4);

    static final int F_LITTLE = 0x0100;
    static final int F_BIG = 0x0200;
    static final int F_AR32WR = 0x0040;
    static final int F_STRP30 = 0x1000;

    /*
     *  Section Header flags
     */
    static final int SHF_WRITE = (1 << 0);      /* Writable */
    static final int SHF_ALLOC = (1 << 1);      /* Occupies memory @ runtime */
    static final int SHF_EXECINSTR = (1 << 2);  /* Executable */
    static final int SHF_MASKPROC = 0xf0000000; /* Processor-specific */

    /*
     *  ======== Target ========
     *  Field mausize:
     *      Number of bytes in minimum addressable unit. 
     *  Field charsize:
     *      Number of bytes in a character. For the C6x, charsize = 1, since
     *      4 characters are packed into a word. For C55x, charsize = 2,
     *      since data memory is only word addressable, although mausize = 1
     *      (program memory is byte addressable).
     */
    static public class Target {
        public int      wordsize;   /* Size of a word */
        public int      mausize;    /* Size of minimum addressable unit */
        public int      charsize;   /* For C55, mausize= 1, but charsize = 2 */
        public int      ptrsize;    /* pointer size; C55L == 4, C55 == 2 */
        public boolean  bigendian;  /* big or little endian? */
        public short    machine;    /* target machine identifier */
        
        public Target(short machine, byte data, int flags) {

            this.machine = machine;
            this.bigendian = data == ELFDATA2MSB ? true : false;
            
            switch (machine) {
                case EM_TI_T16: 
                case EM_TI_C6000: {
                    wordsize = 4;
                    mausize = 1;
                    charsize = 1;
                    ptrsize = 4;
                    break;
                }
                case EM_TI_C5500: {
                    /*  Warning!!! HACK!!! This test was determined by reverse
                     *  engineering the COFF file header and may be wrong.
                     */
                    if ((flags & 0xf0) == 0xe0) {
                        /* C55L */
                        wordsize = 2;
                        mausize = 1;
                        charsize = 2;
                        ptrsize = 4;
                    }
                    else {
                        /* C55 */
                        wordsize = 2;
                        mausize = 1;
                        charsize = 2;
                        ptrsize = 2;
                    }
                    break;
                }

                case EM_386:
                case EM_SPARC:
                case EM_ARM: {
                    wordsize = 4;
                    mausize = 1;
                    charsize = 1;
                    ptrsize = 4;
                    break;
                }
                case EM_TI_C2000: {
                    wordsize = 2;
                    mausize = 2;
                    charsize = 2;
                    ptrsize = 2;
                    break;
                }
                case EM_MSP430: {
                    wordsize = 2;
                    mausize = 1;
                    charsize = 1;
                    ptrsize = 2;
                    break;
                }

                default: {
                    wordsize = 0;
                    mausize = 0;
                    charsize = 0;
                    ptrsize = 0;
                    break;
                }
            }
        }
    }
    
    /*
     *  ======== FileHeader ========
     */
    static public class FileHeader {
        public byte e_ident[] = new byte[EI_NIDENT];
        public short e_type;
        public short e_machine;
        public int e_version;
        public int e_entry;
        public int e_phoff;
        public int e_shoff;
        public int e_flags;
        public short e_ehsize;
        public short e_phentsize;
        public short e_phnum;
        public short e_shentsize;
        public short e_shnum;
        public short e_shstrndx;

        public String  error;
        public boolean swapped = true;

        public void read(RandomAccessFile file)
            throws java.io.IOException
        {
            /* read the file data */
            file.seek(0);

            for (int i = 0; i < EI_NIDENT; i++) {
                e_ident[i] = file.readByte();
            }
            e_type = file.readShort();
            e_machine = file.readShort();
            e_version = file.readInt();
            e_entry = file.readInt();
            e_phoff = file.readInt();
            e_shoff = file.readInt();
            e_flags = file.readInt();
            e_ehsize = file.readShort();
            e_phentsize = file.readShort();
            e_phnum = file.readShort();
            e_shentsize = file.readShort();
            e_shnum = file.readShort();
            e_shstrndx = file.readShort();

            if (e_version < 255) {
                /* e_version should be 1 in almost all cases */
                swapped = false;
            }
            
            if (swapped) {
                e_type = swap(e_type);
                e_machine = swap(e_machine);
                e_version = swap(e_version);
                e_entry = swap(e_entry);
                e_phoff = swap(e_phoff);
                e_shoff = swap(e_shoff);
                e_flags = swap(e_flags);
                e_ehsize = swap(e_ehsize);
                e_phentsize = swap(e_phentsize);
                e_phnum = swap(e_phnum);
                e_shentsize = swap(e_shentsize);
                e_shnum = swap(e_shnum);
                e_shstrndx = swap(e_shstrndx);
            }
        }

        public String toString()
        {
            return ("version = 0x" + Integer.toHexString(e_version)
                + ", machine = 0x" + Integer.toHexString(e_machine & 0xffff)
                + ", nSects = 0x" + Integer.toHexString(e_shnum & 0xffff)
                + ", magic = 0x" + Integer.toHexString(e_ident[0] & 0xffff)
                + (char)e_ident[1] + (char)e_ident[2] + (char)e_ident[3]);
        }
    }

    /*
     *  ======== SectHeader ========
     */
    static public class SectHeader {
        public String name;
        public int sh_name;
        public int sh_type;
        public int sh_flags;
        public int sh_addr;
        public int sh_offset;
        public int sh_size;
        public int sh_link;
        public int sh_info;
        public int sh_addralign;
        public int sh_entsize;

        public void read(RandomAccessFile file, boolean swapped)
            throws java.io.IOException
        {
            sh_name = file.readInt();
            sh_type = file.readInt();
            sh_flags = file.readInt();
            sh_addr = file.readInt();
            sh_offset = file.readInt();
            sh_size = file.readInt();
            sh_link = file.readInt();
            sh_info = file.readInt();
            sh_addralign = file.readInt();
            sh_entsize = file.readInt();

            if (swapped) {
                sh_name = swap(sh_name);
                sh_type = swap(sh_type);
                sh_flags = swap(sh_flags);
                sh_addr = swap(sh_addr);
                sh_offset = swap(sh_offset);
                sh_size = swap(sh_size);
                sh_link = swap(sh_link);
                sh_info = swap(sh_info);
                sh_addralign = swap(sh_addralign);
                sh_entsize = swap(sh_entsize);
            }
        }

        public String getName()
        {
            return (name);
        }

        public String toString()
        {
            return getName()
                + ": base = 0x" + Integer.toHexString(sh_addr)
                + ", size = 0x" + Integer.toHexString(sh_size)
                + " (" + sh_size + ")"
                + ", type = " + sh_type;
        }
    }

    /*
     *  ======== SymbolTableEntry ========
     */
    static public class SymbolTableEntry {
       
        public String name;
        
        public int st_name;
        public int st_value;
        public int st_size;
        public char st_info;
        public char st_other;
        public short st_shndx;      
        
        /*
         *  ======== read ========
         *  Reads a SymbolTableEntry from the current position of the provided
         *  ByteBuffer. Assumes the correct byte order has already been set for
         *  the ByteBuffer.
         */
        public void read(ByteBuffer buffer)
            throws java.io.IOException
        {
            st_name = buffer.getInt();
            st_value = buffer.getInt();
            st_size = buffer.getInt();
            st_info = (char) buffer.get();
            st_other = (char) buffer.get();
            st_shndx = buffer.getShort();            
        }

        /*
         * st_info types as defined by the elf documentation. We're not
         * interested in symbols of these types.
         */
        private static final char STT_NOTYPE  = 0;
        private static final char STT_OBJECT  = 1;
        private static final char STT_FUNC    = 2;
        private static final char STT_SECTION = 3;
        private static final char STT_FILE    = 4;

        /*
         * additional (undocumented?) st_info types as defined by elf.h.
         *
         * FWIW: https://www.freebsd.org/cgi/man.cgi?query=elf%285%29 
         * describes section id SHN_COMMON as:
         *     "Symbols defined relative to this section are common symbols,
         *     such as FORTRAN COMMON or unallocated C external variables."
         */
        private static final char STT_COMMON = 5;  /* Symbol is a common data object */
        private static final char STT_TLS    = 6;  /* Symbol is thread-local data object*/
        private static final char STT_NUM    = 7;  /* the number of types defined by this version of Elf32 */
        private static final char STT_LOOS   = 10; /* Start of OS-specific */
        private static final char STT_HIOS   = 12; /* End of OS-specific */
        private static final char STT_LOPROC = 13; /* Start of processor-specific */
        private static final char STT_HIPROC = 15; /* End of processor-specific */

        /*
         * st_info bindings as defined by the elf documentation.
         */
        private static final char STB_LOCAL  = 0;
        private static final char STB_GLOBAL = 1;
        private static final char STB_WEAK   = 2;
        private static final char STB_LOPROC = 13;
        private static final char STB_HIPROC = 15;

        /*
         *  The st_info values corresponding to data and program symbols (the
         *  types that we're interested in).
         *
         *  st_info is constructed from type and binding values as follows:
         *      ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))
         */
        private static final char LINKER_SYMBOL  = 16; /* global, no type */
        private static final char DATA_SYMBOL    = 17; /* global, object */
        private static final char PROGRAM_SYMBOL = 18; /* global, function */
        private static final char BSS_SYMBOL     = 21; /* global, bss object */

        /*
         *  ======== isAbsolute ========
         */
        boolean isAbsolute()
        {
            if (st_info == (STB_GLOBAL << 4 | STT_NOTYPE)) {
                return (true);
            }

            return (false);
        }

        /*
         *  ======== isData ========
         */
        boolean isData()
        {
            if (st_info == (STB_GLOBAL << 4 | STT_COMMON)
                || st_info == (STB_GLOBAL << 4 | STT_OBJECT)) {
                return (true);
            }

            return (false);
        }

        /*
         *  ======== isCode ========
         */
        boolean isCode()
        {
            if (st_info == (STB_GLOBAL << 4 | STT_FUNC)) {
                return (true);
            }

            return (false);
        }

        /*
         *  ======== isLocalAbsolute ========
         */
        boolean isLocalAbsolute()
        {
            if (st_info == STT_NOTYPE) {
                return (true);
            }

            return (false);
        }

        /*
         *  ======== isLocalCode ========
         */
        boolean isLocalCode()
        {
            if (st_info == STT_FUNC) {
                return (true);
            }

            return (false);
        }

        /*
         *  ======== isLocalData ========
         */
        boolean isLocalData()
        {
            if (st_info == STT_OBJECT) {
                return (true);
            }

            return (false);
        }
    }
    
    /*
     *  ======== getFileHeader ========
     */
    public FileHeader getFileHeader()
    {
        return (fileHeader);
    }

    /*
     *  ======== getSectionCRC ========
     *  Compute CRC for the specified section
     */
    public long getSectionCRC(SectHeader hdr)
        throws java.io.IOException
    {
        return (getSectionCRC(hdr, hdr.sh_addr, hdr.sh_size));
    }

    /*
     *  ======== getSectionCRC ========
     *  Compute CRC for the portion of the section defined by addr and len
     */
    public long getSectionCRC(SectHeader hdr, long addr, int length)
        throws java.io.IOException
    {
        /* convert target addresses into a file offset */
        long delta = addr - hdr.sh_addr;
        if (delta < 0) {
            return (0);
        }
        
        CRC32 crc = new CRC32();
        FileInputStream fis = new FileInputStream(file.getFD());
        BufferedInputStream bis = new BufferedInputStream(fis);
        byte[] byteArr = new byte[1024];

        /* seek to the section in the file */
        long offset = hdr.sh_offset + delta;
        file.seek(offset);

        /* read the data from the file and update CRC */
        int rem = length;
        while (rem > 0) {
            int len = rem > byteArr.length ? byteArr.length : rem;
            if ((len = bis.read(byteArr, 0, len)) <= 0) {
                break;
            }
            crc.update(byteArr, 0, len);
            rem -= len;
        }

        return (crc.getValue());
    }

    /*
     *  ======== getSectionHeader ========
     */
    public SectHeader getSectionHeader(int index)
    {
        if (index >= sectHeaders.length) {
            return (null);
        }
        return (sectHeaders[index]);
    }
    
    /*
     *  ======== getSectionHeaders ========
     */
    public SectHeader[] getSectionHeaders()
    {
        return (sectHeaders);
    }
    
    /*
     *  ======== findString ========
     *  Return the target string located at the specified address
     *
     *  addr is an address as seen by the target; i.e., the value that
     *  would be displayed if the target were to call printf("0x%x", str).
     *
     *  This method only looks in TEXT, DATA or BSS sections and will
     *  return the string located in the first section whose address
     *  range contains the string address; if two sections overlap the
     *  first section in the file will always win.
     *  
     *  This method maintains a cache of the strings found to enable
     *  faster lookup if the string is requested repeatedly. 
     */
    public String findString(long addr)
    {
        String result;
        
        /* 
         * First, check the cache to see if the string has already been 
         * found. 
         */
        if ((result = addrToStr.get(addr)) != null) {
            return (result);
        }
        
        if (target.machine == EM_TI_C5500) {
            addr = addr * 2;    /* C55 data addrs are 1/2 prog mem addrs */
        }

        try {
            for (int i = 0; i < fileHeader.e_shnum; i++) {
                SectHeader header = sectHeaders[i];

                if ((header.sh_flags & SHF_ALLOC) == 0
                    || header.sh_size == 0) {
                    /* skip non-target sections (debug, ...) */
                    continue;
                }

                long start = unsToLong(header.sh_addr);
                long end = start + unsToLong(header.sh_size);
                if (addr >= start && addr < end) {
                    long offset = unsToLong(header.sh_offset)
                        + target.mausize * (addr - start);

                    result = readStringFromFile(offset, target.charsize);
                    
                    /* Add the string to the cache. */
                    addrToStr.put(addr, result);
                    
                    return (result);
                }
            }
        }
        catch (java.io.IOException e) {
            ;
        }
        
        return (null);
    }

    /*
     *  ======== findString ========
     *  Return the target string located at the specified address and section
     *
     *  addr is an address as seen by the target; i.e., the value that
     *  would be displayed if the target were to call printf("0x%x", str).
     *  
     *  This API does not currently implement caching of the located strings.
     */
    public String findString(long addr, String sectionName)
    {
        if (target.machine == EM_TI_C5500) {
            addr = addr * 2;    /* C55 data addrs are 1/2 prog mem addrs */
        }

        SectHeader header = findSectionHeader(sectionName);
        if (header != null && header.sh_size != 0) {
            long start = unsToLong(header.sh_addr);
            long end = start + unsToLong(header.sh_size);
            if (addr >= start && addr < end) {
                try {
                    long offset = unsToLong(header.sh_offset)
                        + target.mausize * (addr - start);
                    return (readStringFromFile(offset, target.charsize));
                }
                catch (java.io.IOException e) {
                    ;
                }
            }
        }

        return (null);
    }

    /*
     *  ======== findSectionHeader ========
     */
    public SectHeader findSectionHeader(String name)
    {
        for (int i = 0; i < fileHeader.e_shnum; i++) {
            if (sectHeaders[i].getName().equals(name)) {
                return (sectHeaders[i]);
            }
        }

        return (null);
    }
    
    /*
     *  ======== getSymbolValue ========
     */
    public long getSymbolValue(String symName)
    {
        Long val = symsByName.get(symName);
        
        if (val == null) {
            return (-1);
        }
        else {
            return (val);
        }
    }
    
    /*
     *  ======== getISA ========
     */
    public int getISA()
    {
        // TODO Auto-generated method stub
        return 0;
    }

    /*
     *  ======== lookupDataSymbol ========
     *  Return either a data or absolute symbol name
     *
     *  We lookup absolute symbols to enable finding symbols delivered in ROM.
     */
    public String[] lookupDataSymbol(long val)
    {
        String [] result = dataSymsByVal.get(val);
        if (result == null || result.length == 0) {
            /* to support ROM, look for a matching absolute */
            result = absSymsByVal.get(val);
        }
        return (result);
    }

    /*
     *  ======== lookupFuncName ========
     *  Return either a program or absolute symbol name
     *
     *  We lookup absolute symbols to enable finding symbols delivered in ROM.
     */
    public String[] lookupFuncName(long val)
    {
        String [] result = progSymsByVal.get(val);
        if (result == null || result.length == 0) {
            /* to support ROM, look for a matching absolute */
            result = absSymsByVal.get(val);
        }
        return (result);
    }

    /*
     *  ======== toString ========
     */
    public String toString()
    {
        String result = "";

        result = fileHeader.toString();
    
        for (int i = 0; i < fileHeader.e_shnum; i++) {
            result = result.concat("\n" + sectHeaders[i].toString());
        }

        return (result);
    }

    /*
     *  ======== close ========
     */
    public void close()
        throws java.io.IOException
    {
        if (file != null) {
            file.close();
            isClosed = true;
        }
    }

    /*
     *  ======== reopen ========
     */
    public void reopen()
        throws java.io.IOException
    {
        if (isClosed) {
            file = new RandomAccessFile(curFileName, "r");
            isClosed = false;
        }
    }
    
    /*
     *  ======== parse ========
     */
    public String parse(String fileName)
        throws java.io.IOException
    {
        String result = "";
        
        if (file != null) {
            file.close();
            isClosed = true;
        }
        if (fileName == null) {
            return ("");
        }

        addrToStr = new HashMap<Long, String>();
        
        file = new RandomAccessFile(fileName, "r");
        curFileName = fileName;
        isClosed = false;
        
        /* read the file header, file if not an Elf32 file */
        fileHeader.read(file);
        if (fileHeader.error != null) {
            file.close();
            return (fileHeader.error);
        }

        /* determine the target, fail if not recognized */
        target = new Target(fileHeader.e_machine,
            fileHeader.e_ident[EI_DATA], fileHeader.e_flags);
        if (target.wordsize == 0) {
            file.close();
            return ("Error: unrecognized architecture (machine = "
                + fileHeader.e_machine + ")");
        }

        /* read each section header */
        sectHeaders = new SectHeader [fileHeader.e_shnum];
        for (int i = 0; i < fileHeader.e_shnum; i++) {
            sectHeaders[i] = new SectHeader();
            file.seek(fileHeader.e_shoff + i * fileHeader.e_shentsize);
            sectHeaders[i].read(file, fileHeader.swapped);
        }

        /* read string table into memory */
        SectHeader strTab = sectHeaders[fileHeader.e_shstrndx];

        /* read section names */
        for (int i = 0; i < fileHeader.e_shnum; i++) {
            long offset = strTab.sh_offset + sectHeaders[i].sh_name;
            sectHeaders[i].name = readStringFromFile(offset, 1);
        }

        return (result);
    }
    
    /*
     *  ======== parseSymbols ========
     *  Parse the elf file for its symbol table.
     *
     *  This function must be called before calling any of the ISymbolTable
     *  methods. It's not necessary to call it this Elf32 object is
     *  only to be used as an IOFReader.
     *
     *  This function assumes 'parse' has already been called.
     */
    public void parseSymbols()
        throws java.io.IOException
    {
        /* Initialize new symbol maps. */
        symsByName = new HashMap<String, Long>();
        dataSymsByVal = new HashMap<Long, String[]>();
        progSymsByVal = new HashMap<Long, String[]>();
        absSymsByVal = new HashMap<Long, String[]>();
        
        /* Retrieve the section containing the symbol table entries. */
        // TODO - Are these sections always identified as ".symtab" and  
        //        ".strtab"? Should I be looking at section header type 
        //        instead?
        SectHeader symTabHdr = this.findSectionHeader(".symtab");
        ByteBuffer symTabBuffer = readSectionBytes(symTabHdr);
        
        /* Retrieve the section containing all of the symbol names. */
        SectHeader symStrTabHdr = this.findSectionHeader(".strtab");        
        ByteBuffer symStrTabBuffer = readSectionBytes(symStrTabHdr);

        /* 
         * Set the appropriate endianess for the ByteBuffer. 
         * The default is big endian.
         */
        if (fileHeader.swapped) {
            symTabBuffer.order(java.nio.ByteOrder.LITTLE_ENDIAN);
            
            /* 
             * The strings are only read as bytes, so it's not technically
             * necessary to set the byte order for this buffer.
             */
            symStrTabBuffer.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        }
        
        /*
         * TODO - Possible code for finding the symbol table by type.
         */
        /*
        for (int i = 0; i < fileHeader.e_shnum; i++) {
            int type = sectHeaders[i].sh_type;
            if (type == 2) {
                // This finds the ".symtab" section.
                System.out.println("Found a SHT_SYMTAB section! " + sectHeaders[i].getName());
            }
            else if (type == 11) {
                System.out.println("Found a SHT_DYNSYM section! " + sectHeaders[i].getName());
            }
            else if (type == 5) {
                System.out.println("Section has a symbol hash table!");
                System.out.println("  Section name: " + sectHeaders[i].getName());
            }
        }
        */
        
        /* Determine the number of symbols in the table. */
        int numSymbols = symTabHdr.sh_size / symTabHdr.sh_entsize;

        /* For each of the symbols in the table. */
        for (int i = 0; i < numSymbols; i++) {

            /* Create a SymbolTableEntry object to represent the entry. */
            SymbolTableEntry entry = new SymbolTableEntry();
            
            /* Parse the next entry. */
            entry.read(symTabBuffer);

            //            System.out.println("    " + readStringFromBuffer(symStrTabBuffer, entry.st_name, 1)
            //                + ", st_shndx: 0x" + Integer.toHexString((int)entry.st_shndx)
            //                + ", st_info: 0x" + Integer.toHexString((int)entry.st_info)
            //                + ", st_size: 0x" + Integer.toHexString((int)entry.st_size)
            //                + ", st_other: 0x" + Integer.toHexString((int)entry.st_other));

            /* If this is a data or program symbol ... */
            if (entry.isAbsolute() || entry.isLocalAbsolute()
                || entry.isData() || entry.isLocalData()
                || entry.isCode() || entry.isLocalCode()) {
                                
                /* Read the symbol name from the file. */
                // TODO - Are the symbol table strings always encoded as
                //        1 byte-per-char, or should we use the target size?
                entry.name = 
                    readStringFromBuffer(symStrTabBuffer, entry.st_name, 1);

                /* skip code symbols that are not valid C/C++ identifiers */
                if (entry.isLocalCode() || entry.isLocalAbsolute()) {
                    if (!cIdent.reset(entry.name).matches()) {
                        continue;
                    }
                }

                /* Retrieve the value of the symbol. */
                long val = entry.st_value; 
                
                /* Take care of sign extension. */
                if (val < 0) {
                    val += Math.pow(2, 32);
                }
                
                /* Add the symbol to the map
                 * (never overwrite global with to local symbol names)
                 */
                if (symsByName.get(entry.name) == null
                    || !entry.isLocalData()) {
                    symsByName.put(entry.name, val);
                }
                
                /* 
                 * Determine which value-to-name map to add the symbol to
                 * (based on whether it's a data or program symbol).
                 */
                HashMap<Long, String[]> symsByVal;
                if (entry.isData() || entry.isLocalData()) {
                    symsByVal = dataSymsByVal;
                }
                else if (entry.isCode() || entry.isLocalCode()) {
                    symsByVal = progSymsByVal;
                }
                else if (entry.isAbsolute() || entry.isLocalAbsolute()) {
                    symsByVal = absSymsByVal;
                }
                else {
                    continue;
                }
                
                /* Check to see if the address already exists in the map. */
                String[] syms = symsByVal.get(val);
                
                String[] newSyms;
                
                /* 
                 * If there are already symbols in the map at 
                 * this address... 
                 */
                if (syms != null) {
                    /* 
                     * Create a new array to hold the existing symbols
                     * plus this new one.
                     */
                    newSyms = new String[syms.length + 1];
                    
                    /* Copy over the existing symbols. */
                    System.arraycopy(syms, 0, newSyms, 0, syms.length);
                    
                    /* Add the new symbol to the end. */
                    newSyms[newSyms.length - 1] = demangle(entry.name);
                }
                /* 
                 * Otherwise, if this is the first symbol at 
                 * this address...
                 */
                else {
                    newSyms = new String[1];
                    
                    newSyms[0] = demangle(entry.name);
                }
                
                /* Add the symbols to the map. */
                symsByVal.put(val, newSyms);
            }
            else {
                //System.out.println("Skipping: " + strPad(readStringFromBuffer(symStrTabBuffer, entry.st_name, 1), 25) + " type: " + (new Integer(entry.st_info)).toString());
            }
        }
        
        /* Make the symbol mappings that we generate. */
        // TODO - This info must be available somewhere in the elf file?
        
        /* Map _stack to __stack. */
        long addr = getSymbolValue("_stack");
        if (addr != -1) {
            symsByName.put("__stack", addr);
        }
        /* If _stack isn't found, try __TI_STACK_BASE. */
        else {
            addr = getSymbolValue("__TI_STACK_BASE");
            if (addr != -1) {
                symsByName.put("__stack", addr);
            }
        }
        
        /* Map "__TI_STACK_SIZE" to "__STACK_SIZE". */ 
        addr = getSymbolValue("__TI_STACK_SIZE");
        if (addr != -1) {
            symsByName.put("__STACK_SIZE", addr);
        }
    }

    /*
     *  ======== getGlobalVariables ========
     *  Get all the global variables and their type information
     */
    public Dwarf32.Variable[] getGlobalVariables()
        throws Exception
    {
        return (getGlobalVariablesByType(".*"));
    }

    /*
     *  ======== getGlobalVariablesByType ========
     *  Get global variables of particular types matching the regex
     *
     *  @params(typeRegEx)  Regular expression to limit types of variables
     */
    public Dwarf32.Variable[] getGlobalVariablesByType(String typeRegEx)
        throws Exception
    {
        SectHeader infoHeader, abbrevHeader, strHeader;
        ByteBuffer info, abbrev;
        ByteBuffer str = null;

        /* Read all the sections */
        infoHeader = findSectionHeader(".debug_info");
        abbrevHeader = findSectionHeader(".debug_abbrev");

        if (infoHeader == null) {
            throw new Exception("Cannot find .debug_info");
        }

        if (abbrevHeader == null) {
            throw new Exception("Cannot find .debug_abbrev");
        }

        info = readSectionBytes(infoHeader);
        abbrev = readSectionBytes(abbrevHeader);
        if (fileHeader.swapped) {
            info.order(java.nio.ByteOrder.LITTLE_ENDIAN);
            abbrev.order(java.nio.ByteOrder.LITTLE_ENDIAN);
        }

        /* Some codegen tools (like IAR) do not have string section */
        strHeader = findSectionHeader(".debug_str");
        if (strHeader != null) {
            str = readSectionBytes(strHeader);

            if (fileHeader.swapped) {
                str.order(java.nio.ByteOrder.LITTLE_ENDIAN);
            }
        }

        Dwarf32 dw = new Dwarf32();
        dw.parse(info, abbrev, str);

        Dwarf32.Variable[] varArr = dw.getGlobalVariablesByType(typeRegEx);
        ArrayList<Dwarf32.Variable> varList = new ArrayList<Dwarf32.Variable>();

        parseSymbols();

        /* Filter out all the non existent dwarf variables */
        for (int i = 0; i < varArr.length; i++) {
            String sym = varArr[i].name;
            if (varArr[i].name.indexOf('.') != -1) {
                sym = varArr[i].name.substring(
                         0, varArr[i].name.indexOf('.'));
            }
            if (getSymbolValue(sym) != -1) {
                varList.add(varArr[i]);
            }
        }

        return (varList.toArray(new Dwarf32.Variable[varList.size()]));
    }

    public static String strPad(String msg, int pad)
    {
        while (msg.length() < pad) {
            msg += " ";
        }
        return (msg);
    }

    /*
     *  ======== readSectionBytes ========
     *  Reads an entire section from the elf file, and returns it as a
     *  ByteBuffer.
     *
     *  The file is read in sections of 10MB max to avoid OutOfMemoryErrors for
     *  large files.
     */
    public ByteBuffer readSectionBytes(SectHeader hdr)
        throws java.io.IOException
    {
        byte[] byteArr = new byte[0x800000];
        FileChannel fc = file.getChannel();

        /* Seek to the section. */
        file.seek(hdr.sh_offset);

        ByteBuffer smallBuffer = ByteBuffer.wrap(byteArr);
        ByteBuffer largeBuffer = ByteBuffer.allocate(hdr.sh_size);

        int counter = 0;
        do {
            smallBuffer.position(0);
            fc.read(smallBuffer);
            if (smallBuffer.position() > 0) {
                /* read from a small buffer to a large buffer */
                smallBuffer.flip();

                /* have to be careful not to go over the size of the section */
                int limit = smallBuffer.limit();
                if (counter + limit > hdr.sh_size) {
                    limit = hdr.sh_size - counter;
                }
                largeBuffer.put(byteArr, 0, limit);
                counter = counter + limit;
            }
        } while (counter < hdr.sh_size);

        largeBuffer.position(0);
        return (largeBuffer);
    }

    /*
     *  ======== readSection ========
     *  Reads bytes from a section in the elf file
     */
    public int readSection(SectHeader hdr, byte[] byteArr, long addr, int len)
        throws java.io.IOException
    {
        /* never read more than what fits in byteArr */
        if (len > byteArr.length) {
            len = byteArr.length;
        }
        
        /* convert target addresses into a file offset */
        long delta = addr - hdr.sh_addr;
        if (delta < 0) {
            return (0);
        }
        
        /* seek to the section in the file */
        long offset = hdr.sh_offset + delta;
        file.seek(offset);

        /* read the data from the file */
        return (file.read(byteArr, 0, len));
    }
    
    /*
     *  ======== decimate ========
     */
    private int decimate(byte [] stringBuf, int df)
    {
        int len = stringBuf.length / df;
        int pos = target.bigendian ? (df - 1) : 0;

        for (int i = (target.bigendian ? 0 : 1); i < len; i++) {
            stringBuf[i] = stringBuf[df * i + pos];
        }

        return (len);
    }

    /*
     *  ======== demangle ========
     *  TI mangles static identifies when build with whole program
     *  optimization (by adding "$<file number>" to static symbols).  To hide
     *  this hack, we must remove the suffix when clients aare looking up the
     *  name corresponding to an address.
     */
    private String demangle(String name)
    {
        if (name != null) {
            int k = name.indexOf('$');
            if (k != -1) {
                name = name.substring(0, k);
            }
        }
        return (name);
    }

    /*
     *  ======== readStringFromBuffer ========
     *  Reads the string from 'buffer' starting at 'offset'.
     *
     *  This function is simpler than 'readStringFromFile' because all of the
     *  characters have already been read into memory. 
     */
    private String readStringFromBuffer(ByteBuffer buffer, int offset, int df)
    {
        /* Move the ByteBuffer's position to the specified offset. */
        buffer.position(offset);
        
        /* Walk through the buffer until we find the null terminator. */
        while (buffer.get() != 0) {
            ;
        }
        
        /* 
         * Calculate the length of the string based on the current position 
         * (the char after the null terminator). 
         */
        int strLen = buffer.position() - offset - 1;
        
        /* Move the buffer back to the location of the string. */
        buffer.position(offset);
        
        /* Allocate a byte[] to hold the string. */
        byte [] stringBuf = new byte[strLen];
        
        /* Copy the characters from the ByteBuffer into stringBuf. */
        buffer.get(stringBuf, 0, strLen);
        
        /* Handle targets where character is more than one byte. */
        if (df > 1) {
            strLen = decimate(stringBuf, df);
        }
        
        /* Create a String from the buffer and return it. */
        return (new String(stringBuf, 0, strLen));        
    }
    
    /*
     *  ======== readStringFromFile ========
     */
    private String readStringFromFile(long offset, int df)
        throws java.io.IOException
    {
        boolean done = false;
        String result = "";

        if (isClosed) {
            file = new RandomAccessFile(curFileName, "r");
        }
        file.seek(offset);

//        System.out.println("string at offset 0x" + Long.toHexString(offset));
        
        while (!done) {
            byte [] stringBuf = new byte[80];
            int n = file.read(stringBuf);
            if (n <= 0) {
                break;
            }

            if (df > 1) {
                n = decimate(stringBuf, df);
            }
            
            for (int i = 0; i < n; i++) {
                if (stringBuf[i] == 0) {
                    n = i;
                    done = true;
                    break;
                }
            }
            result = result.concat(new String(stringBuf, 0, n));
        }

        if (isClosed) {
            file.close();
        }

        return (result);
    }

    /*
     *  ======== swap ========
     */
    static short swap(short value)
    {
        int b1 = value & 0xff;
        int b2 = (value >> 8) & 0xff;
        
        return ((short)(b1 << 8 | b2));
    }

    static int swap(int value)
    {
        int b1 = (value >>  0) & 0xff;
        int b2 = (value >>  8) & 0xff;
        int b3 = (value >> 16) & 0xff;
        int b4 = (value >> 24) & 0xff;
        
        return (b1 << 24 | b2 << 16 | b3 << 8 | b4);
    }

    /*
     *  ======== unsToLong ========
     */
    private static long unsToLong(int value)
    {
        long res = value;
        return (((res << 32) >>> 32));
    }

    /* HashMap for caching the strings found by 'findString(addr)' */
    private HashMap<Long, String> addrToStr;
    
    /* HashMaps for storing the symbol table. */
    private HashMap<String, Long> symsByName;
    private HashMap<Long, String[]> dataSymsByVal;
    private HashMap<Long, String[]> progSymsByVal;
    private HashMap<Long, String[]> absSymsByVal;
    
    private RandomAccessFile file;
    private String curFileName = null;
    private boolean isClosed = true;
    private FileHeader fileHeader = new FileHeader();
    private SectHeader [] sectHeaders;
    private int fileHeaderSize = 0;
    private long stringTabOffset = 0;
    private Target target;
    
    /*
     *  ======== main ========
     *  Unit test for this class
     */
    public static void main(String args[])
    {
        Elf32 elf = new Elf32();

        try {
            for (int i = 0; i < args.length - 1; i += 2) {
                /* read Elf file */
                System.out.println(args[i] + " ...");
                String error = elf.parse(args[i]);
                if (error != "") {
                    System.out.println("Error: can't parse " + args[i]
                        + ": " + error);
                    continue;
                }

                /* display elf file */
                System.out.println(elf.toString());

                /* Parse the symbol table before calling symbol APIs. */
                elf.parseSymbols();
                
                /* Lookup selected symbols */
                String[] snames = new String [] {
                    "semStruct",
                    "xdc_runtime_Memory_Module__state__V",
                    "ti_sysbios_knl_Queue_Module__root__V",
                    "ti_sysbios_knl_Task_Module__root__V",
                    "__stack"
                };
                long val = 0;
                for (int j = 0; j < snames.length; j++) {
                    val = elf.getSymbolValue(snames[j]);
                    System.out.println("Looking up '" + snames[j] + "': 0x" + 
                                       Long.toHexString(val));
                }
                
                /* Look up an address. */
                System.out.println("Looking up addr 0x" + Long.toHexString(val) + ": ");
                String[] syms = elf.lookupDataSymbol(val);
                for (int j = 0; j < syms.length; j++) {
                    System.out.println("  " + syms[j]);
                }
                
                /* lookup a string from an address */
                System.out.println("Looking up string at addr " + args[i + 1] + ": ");
                long addr = Long.decode(args[i + 1]);
                String value = elf.findString(addr);
                if (value == null) {
                    System.out.println("Error: can't find string at "
                        + args[i + 1]);
                }
                else {
                    System.out.println('"' + value + '"');
                }

                /* look for string in the .rodata section */
                value = elf.findString(addr, ".rodata");
                if (value == null) {
                    System.out.println("can't find string at "
                        + args[i + 1] + " within the .rodata section");
                }
                else {
                    System.out.println(".rodata: " + value);
                }

                /* look for string in the .const section */
                value = elf.findString(addr, ".const");
                if (value == null) {
                    System.out.println("Error: can't find string at "
                        + args[i + 1] + " within the .const section");
                }
                else {
                    System.out.println(".const: " + value);
                }
            }

            /* close any open file associated with elf */
            elf.close();
        }
        catch (java.io.IOException e) {
            System.err.println("" + e);
        }
    }
}
