/*
 *  Copyright 2017 by Texas Instruments Incorporated.
 *
 */

/*
 *  ======== elf.c ========
 */

#include <xdc/std.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ti/targets/omf/elf/elf.h>

typedef struct Elf32_Object {
    Elf32_Ehdr header;
    FILE *file;
    Char *strTab;
    Bool readOnly;
} Elf32_Object;

/*
 *  ======== Elf32_create ========
 *  Create elf file handle
 *
 *  Elf files have the following format:
 *      ELF Header
 *          e_shoff (offset to section headers) ----+
 *          e_shnum (number of section headers)     |
 *          e_shentsize (size of section header)    |
 *      Program Header Table (opt)                  |
 *      Section 1 <-------------+                   |
 *        :                     |                   |
 *      Section Header Table <----------------------+
 *          Section header 0    |
 *              sh_offset ------+       
 *              sh_size
 *                 :
 *          Section header 1
 *              :
 *
 *  Only the ELF header is in a fixed location, all other sections can
 *  appear in any order.
 *
 *  All section headers have the same size (specified in the ELF header).
 *
 *  Sections satisfy several conditions:
 *     1. Every section in an object file has exactly one section header
 *        describing it. Section headers may exist that do not have a section.
 *     2. Each section occupies one contiguous (possibly empty) sequence of
 *        bytes within a file.
 *     3. Sections in a file may not overlap. No byte in a file resides in
 *        more than one section.
 *     4. An object file may have inactive space. The various headers and
 *        the sections might not "cover" every byte in an object file.
 *        The contents of the inactive data are unspecified.
 */
Elf32_Handle Elf32_create(String fileName, String *status)
{
    Elf32_Handle elf;
    String dummy;
    
    if (status == NULL) {
        status = &dummy;
    }
    
    if ((elf = (Elf32_Object *)malloc(sizeof(struct Elf32_Object))) != NULL) {
        elf->strTab = NULL;
        elf->file = NULL;
        elf->readOnly = FALSE;
        
        /* read the ELF header */
        if ((elf->file = fopen(fileName, "r+b")) == NULL) {
            elf->readOnly = TRUE;
            if ((elf->file = fopen(fileName, "rb")) == NULL) {
                *status = "can't open file";
                goto exit;
            }
        }

        if (fread(&elf->header, sizeof(elf->header), 1, elf->file) != 1) {
            *status = "can't read header";
            goto exit;
        }
    
        /* check magic number in the header */
        if (elf->header.e_ident[Elf32_EI_MAG0] != Elf32_ELFMAG0
            || elf->header.e_ident[Elf32_EI_MAG1] != Elf32_ELFMAG1
            || elf->header.e_ident[Elf32_EI_MAG2] != Elf32_ELFMAG2
            || elf->header.e_ident[Elf32_EI_MAG3] != Elf32_ELFMAG3) {

            *status = "bad magic number";
            goto exit;
        }
        return (elf);
    }

exit:
    Elf32_delete(elf);
    return (NULL);
}

/*
 *  ======== Elf32_delete ========
 */
Void Elf32_delete(Elf32_Handle elf)
{
    if (elf != NULL) {
        if (elf->file != NULL) {
            fflush(elf->file);
            fclose(elf->file);
        }
        if (elf->strTab != NULL) {
            free(elf->strTab);
        }
        free(elf);
    }
}

/*
 *  ======== Elf32_getSize ========
 */
String Elf32_getSize(Elf32_Handle elf, Long *size)
{
    Elf32_Shdr sheader;
    Long max;
    Int i;
    
    *size = 0L;
    
    /* search for end of the file: the end of the last section */
    max = elf->header.e_shoff + elf->header.e_shentsize * elf->header.e_shnum;
    fseek(elf->file, elf->header.e_shoff, SEEK_SET);
    for (i = 0; i < elf->header.e_shnum; i++) {
        Long tmp;
        if (fread(&sheader, sizeof(sheader), 1, elf->file) != 1) {
            return ("can't read elf file section");
        }
        tmp = sheader.sh_offset + sheader.sh_size;
        if (tmp > max) {
            max = tmp;
        }
    }
    *size = max;
    
    return (NULL);
}


/*
 *  ======== Elf32_getEhdr ========
 */
String Elf32_getEhdr(Elf32_Handle elf, Elf32_Ehdr *ehdr)
{
    *ehdr = elf->header;
    
    return (NULL);
}

/*
 *  ======== Elf32_getShdr ========
 */
String Elf32_getShdr(Elf32_Handle elf, Int sNum, Elf32_Shdr *shdr)
{
    Elf32_Shdr sheader;
    Elf32_Off offset;
    
    if (sNum < 0 || sNum >= elf->header.e_shnum) {
        return ("invalid section number");
    }
    
    offset = elf->header.e_shoff + sNum * sizeof(Elf32_Shdr); 
    fseek(elf->file, offset, SEEK_SET);

    if (fread(&sheader, sizeof(sheader), 1, elf->file) != 1) {
        return ("can't read section header");
    }

    *shdr = sheader;
    
    return (NULL);
}

/*
 *  ======== Elf32_getShdrByName ========
 */
String Elf32_getShdrByName(Elf32_Handle elf, String name,
    Elf32_Shdr *shdr, Int *sNum)
{
    Elf32_Shdr sheader;
    Int i;
    
    /* read section header string table (if necessary) */
    if (elf->strTab == NULL) {
        elf->strTab = Elf32_readSect(elf, elf->header.e_shstrndx);
        if (elf->strTab == NULL) {
            return ("can't read string table");
        }
    }
    
    /* search for end of the file: the end of the last section */
    fseek(elf->file, elf->header.e_shoff, SEEK_SET);
    for (i = 0; i < elf->header.e_shnum; i++) {
        if (fread(&sheader, sizeof(sheader), 1, elf->file) != 1) {
            return ("can't read elf file section");
        }
        if (strcmp(sheader.sh_name + elf->strTab, name) == 0) {
            *sNum = i;
            break;
        }
    }
    if (i >= elf->header.e_shnum) {
        return ("can't find named section");
    }

    *shdr = sheader;
    
    return (NULL);
}

/*
 *  ======== Elf32_getSName ========
 */
String Elf32_getSName(Elf32_Handle elf, Elf32_Shdr *shdr)
{
    /* read section header string table (if necessary) */
    if (elf->strTab == NULL) {
        elf->strTab = Elf32_readSect(elf, elf->header.e_shstrndx);
        if (elf->strTab == NULL) {
            return ("can't read string table");
        }
    }
    
    return (shdr->sh_name + elf->strTab);
}

/*
 *  ======== Elf32_read ========
 */
Bool Elf32_read(Elf32_Handle elf, Elf32_Off offset, Char *buf, SizeT len)
{
    fseek(elf->file, offset, SEEK_SET);
    if (fread(buf, len, 1, elf->file) != 1) {
        return (FALSE);
    }
    
    return (TRUE);
}

/*
 *  ======== Elf32_readSect ========
 */
Char *Elf32_readSect(Elf32_Handle elf, Int sNum)
{
    Elf32_Shdr sheader;
    Char *result = NULL;
    Elf32_Off offset;
    
    if (sNum < 0 || sNum >= elf->header.e_shnum) {
        return (NULL);
    }

    offset = elf->header.e_shoff + sNum * sizeof(Elf32_Shdr); 
    fseek(elf->file, offset, SEEK_SET);
    if (fread(&sheader, sizeof(sheader), 1, elf->file) != 1) {
        return (NULL);
    }
    
    result = (Char *)malloc(sheader.sh_size * sizeof (Char));
    if (result != NULL) {
        fseek(elf->file, sheader.sh_offset, SEEK_SET);
        if (fread(result, sheader.sh_size, 1, elf->file) != 1) {
            free(result);
            result = NULL;
        }
    }

    return (result);
}

/*
 *  ======== Elf32_redefineSect ========
 */
String Elf32_redefineSect(Elf32_Handle elf, Int sNum, String fileName)
{
    Elf32_Shdr sheader;
    String result = NULL;
    FILE *new;
    Int ch;
    Long len;
    Elf32_Off offset;
    
    if (elf->readOnly) {
        return ("can't modify readonly ELF file");
    }
    
    /* read original section header */
    offset = elf->header.e_shoff + sNum * sizeof(Elf32_Shdr); 
    fseek(elf->file, offset, SEEK_SET);
    if (fread(&sheader, sizeof(sheader), 1, elf->file) != 1) {
        return ("can't read section header");
    }
    
    /* open and append new section data to the end of the ELF file */
    if ((new = fopen(fileName, "rb")) == NULL) {
        return ("can't open file");
    }

    /* get length of new section */
    fseek(new, 0L, SEEK_END);
    len = ftell(new);
    fseek(new, 0L, SEEK_SET);

    /* if it fits, copy in place; otherwise append to end of file */
    if (len > (Long)sheader.sh_size) {
        fseek(elf->file, 0L, SEEK_END);
        sheader.sh_offset = ftell(elf->file);
    }
    else {
        fseek(elf->file, sheader.sh_offset, SEEK_SET);
    }
    sheader.sh_size = len;

    while ((ch = fgetc(new)) != EOF) {
        fputc(ch, elf->file);
    }

    fclose(new);

    /* re-write section offset and length in section header */
    fseek(elf->file, offset, SEEK_SET);
    if (fwrite(&sheader, sizeof(sheader), 1, elf->file) != 1) {
        return ("can't write section header");
    }

    return (result);
}

/*
 *  ======== Elf32_writeSect ========
 */
Int Elf32_writeSect(Elf32_Handle elf, Int sNum, Char *buf, Int len)
{
    Elf32_Shdr sheader;
    Elf32_Off offset;
    
    if (sNum < 0 || sNum >= elf->header.e_shnum) {
        return (-1);
    }

    offset = elf->header.e_shoff + sNum * sizeof(Elf32_Shdr); 
    fseek(elf->file, offset, SEEK_SET);
    if (fread(&sheader, sizeof(sheader), 1, elf->file) != 1) {
        return (-1);
    }
    
    if (len <= 0) {
        len = sheader.sh_size;
    }
    
    fseek(elf->file, sheader.sh_offset, SEEK_SET);
    return (fwrite(buf, 1, len, elf->file));
}
/*
 *  @(#) ti.targets.omf.elf; 1,0,0,0; 2-10-2017 09:26:13; /db/ztree/library/trees/xdctargets/xdctargets-m11/src/ xlibrary

 */

