package ti.targets.omf.elf;

import java.io.RandomAccessFile;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

/*
 *  ======== Dwarf32 ========
 */
public class Dwarf32
{
    /*
     *  ======== attributes ========
     */
    private static final int DW_AT_sibling = 0x01; /* reference */
    private static final int DW_AT_location = 0x02; /* block, loclistptr */
    private static final int DW_AT_name = 0x03; /* string */

    private static final int DW_AT_ordering = 0x09; /* constant */

    private static final int DW_AT_byte_size = 0x0b; /* block, constant, reference */
    private static final int DW_AT_bit_offset = 0x0c; /* block, constant, reference */
    private static final int DW_AT_bit_size = 0x0d; /* block, constant, reference */

    private static final int DW_AT_stmt_list = 0x10; /* lineptr */
    private static final int DW_AT_low_pc = 0x11; /* address */
    private static final int DW_AT_high_pc = 0x12; /* address */
    private static final int DW_AT_language = 0x13; /* constant */

    private static final int DW_AT_discr = 0x15; /* reference */
    private static final int DW_AT_discr_value = 0x16; /* constant */
    private static final int DW_AT_visibility = 0x17; /* constant */
    private static final int DW_AT_import = 0x18; /* reference */
    private static final int DW_AT_string_length = 0x19; /* block, loclistptr */
    private static final int DW_AT_common_reference = 0x1a; /* reference */
    private static final int DW_AT_comp_dir = 0x1b; /* string */
    private static final int DW_AT_const_value = 0x1c; /* block, constant, string */
    private static final int DW_AT_containing_type = 0x1d; /* reference */
    private static final int DW_AT_default_value = 0x1e; /* reference */

    private static final int DW_AT_inline = 0x20; /* constant */
    private static final int DW_AT_is_optional = 0x21; /* flag */
    private static final int DW_AT_lower_bound = 0x22; /* block, constant, reference */

    private static final int DW_AT_producer = 0x25; /* string */

    private static final int DW_AT_prototyped = 0x27; /* flag */

    private static final int DW_AT_return_addr = 0x2a; /* block, loclistptr */

    private static final int DW_AT_start_scope = 0x2c; /* constant */

    private static final int DW_AT_bit_stride = 0x2e; /* constant */
    private static final int DW_AT_upper_bound =  0x2f; /* block, constant, reference */

    private static final int DW_AT_abstract_origin = 0x31; /* reference */
    private static final int DW_AT_accessibility = 0x32; /* constant */
    private static final int DW_AT_address_class = 0x33; /* constant */
    private static final int DW_AT_artificial = 0x34; /* flag */
    private static final int DW_AT_base_types = 0x35; /* reference */
    private static final int DW_AT_calling_convention = 0x36; /* constant */
    private static final int DW_AT_count = 0x37; /* block, constant, reference */
    private static final int DW_AT_data_member_location = 0x38; /* block, constant, loclistptr */
    private static final int DW_AT_decl_column =  0x39; /* constant */
    private static final int DW_AT_decl_file = 0x3a; /* constant */
    private static final int DW_AT_decl_line = 0x3b; /* constant */
    private static final int DW_AT_declaration = 0x3c; /* flag */
    private static final int DW_AT_discr_list = 0x3d; /* block */
    private static final int DW_AT_encoding = 0x3e; /* constant */
    private static final int DW_AT_external = 0x3f; /* flag */
    private static final int DW_AT_frame_base = 0x40; /* block, loclistptr */
    private static final int DW_AT_friend = 0x41; /* reference */
    private static final int DW_AT_identifier_case = 0x42; /* constant */
    private static final int DW_AT_macro_info = 0x43; /* macptr */
    private static final int DW_AT_namelist_item = 0x44; /* block */
    private static final int DW_AT_priority = 0x45; /* reference */
    private static final int DW_AT_segment = 0x46; /* block, loclistptr */
    private static final int DW_AT_specification = 0x47; /* reference */
    private static final int DW_AT_static_link = 0x48; /* block, loclistptr */
    private static final int DW_AT_type = 0x49; /* reference */
    private static final int DW_AT_use_location = 0x4a; /* block, loclistptr */
    private static final int DW_AT_variable_parameter = 0x4b; /* flag */
    private static final int DW_AT_virtuality = 0x4c; /* constant */
    private static final int DW_AT_vtable_elem_location = 0x4d; /* block, loclistptr */
    private static final int DW_AT_allocated = 0x4e; /* block, constant, reference */
    private static final int DW_AT_associated = 0x4f ;/* block, constant, reference */
    private static final int DW_AT_data_location = 0x50; /* block */
    private static final int DW_AT_byte_stride = 0x51; /* block, constant, reference */
    private static final int DW_AT_entry_pc = 0x52; /* address */
    private static final int DW_AT_use_UTF8 = 0x53; /* flag */
    private static final int DW_AT_extension = 0x54; /* reference */
    private static final int DW_AT_ranges = 0x55; /* rangelistptr */
    private static final int DW_AT_trampoline = 0x56; /* address, flag, reference, string */
    private static final int DW_AT_call_column = 0x57; /* constant */
    private static final int DW_AT_call_file = 0x58; /* constant */
    private static final int DW_AT_call_line = 0x59; /* constant */
    private static final int DW_AT_description = 0x5a; /* string */
    private static final int DW_AT_binary_scale = 0x5b; /* constant */
    private static final int DW_AT_decimal_scale = 0x5c; /* constant */
    private static final int DW_AT_small = 0x5d; /* reference */
    private static final int DW_AT_decimal_sign = 0x5e; /* constant */
    private static final int DW_AT_digit_count = 0x5f; /* constant */
    private static final int DW_AT_picture_string = 0x60; /* string */
    private static final int DW_AT_mutable = 0x61; /* flag */
    private static final int DW_AT_threads_scaled = 0x62; /* flag */
    private static final int DW_AT_explicit = 0x63; /* flag */
    private static final int DW_AT_object_pointer = 0x64; /* reference */
    private static final int DW_AT_endianity = 0x65; /* constant */
    private static final int DW_AT_elemental = 0x66; /* flag */

    private static final int DW_AT_lo_user = 0x2000;
    private static final int DW_AT_hi_user = 0x3fff;

    /*
     *  ======== tags ========
     */
    private static final int DW_TAG_array_type = 0x01;
    private static final int DW_TAG_class_type = 0x02;
    private static final int DW_TAG_entry_point = 0x03;
    private static final int DW_TAG_enumeration_type = 0x04;
    private static final int DW_TAG_formal_parameter = 0x05;
    private static final int DW_TAG_imported_declaration = 0x08;
    private static final int DW_TAG_label = 0x0a;
    private static final int DW_TAG_lexical_block = 0x0b;
    private static final int DW_TAG_member = 0x0d;
    private static final int DW_TAG_pointer_type = 0x0f;
    private static final int DW_TAG_reference_type = 0x10;
    private static final int DW_TAG_compile_unit = 0x11;
    private static final int DW_TAG_string_type = 0x12;
    private static final int DW_TAG_structure_type = 0x13;
    private static final int DW_TAG_subroutine_type = 0x15;
    private static final int DW_TAG_typedef = 0x16;
    private static final int DW_TAG_union_type = 0x17;
    private static final int DW_TAG_unspecified_parameters = 0x18;
    private static final int DW_TAG_variant = 0x19;
    private static final int DW_TAG_common_block = 0x1a;
    private static final int DW_TAG_common_inclusion = 0x1b;
    private static final int DW_TAG_inheritance = 0x1c;
    private static final int DW_TAG_inlined_subroutine = 0x1d;
    private static final int DW_TAG_module = 0x1e;
    private static final int DW_TAG_ptr_to_member_type = 0x1f;
    private static final int DW_TAG_set_type = 0x20;
    private static final int DW_TAG_subrange_type = 0x21;
    private static final int DW_TAG_with_stmt = 0x22;
    private static final int DW_TAG_access_declaration = 0x23;
    private static final int DW_TAG_base_type = 0x24;
    private static final int DW_TAG_catch_block = 0x25;
    private static final int DW_TAG_const_type = 0x26;
    private static final int DW_TAG_constant = 0x27;
    private static final int DW_TAG_enumerator = 0x28;
    private static final int DW_TAG_file_type = 0x29;
    private static final int DW_TAG_friend = 0x2a;
    private static final int DW_TAG_namelist = 0x2b;
    private static final int DW_TAG_namelist_item = 0x2c;
    private static final int DW_TAG_packed_type = 0x2d;
    private static final int DW_TAG_subprogram = 0x2e;
    private static final int DW_TAG_template_type_parameter = 0x2f;
    private static final int DW_TAG_template_value_parameter = 0x30;
    private static final int DW_TAG_thrown_type = 0x31;
    private static final int DW_TAG_try_block = 0x32;
    private static final int DW_TAG_variant_part = 0x33;
    private static final int DW_TAG_variable = 0x34;
    private static final int DW_TAG_volatile_type = 0x35;
    private static final int DW_TAG_dwarf_procedure = 0x36;
    private static final int DW_TAG_restrict_type = 0x37;
    private static final int DW_TAG_interface_type = 0x38;
    private static final int DW_TAG_namespace = 0x39;
    private static final int DW_TAG_imported_module = 0x3a;
    private static final int DW_TAG_unspecified_type = 0x3b;
    private static final int DW_TAG_partial_unit = 0x3c;
    private static final int DW_TAG_imported_unit = 0x3d;
    private static final int DW_TAG_condition = 0x3f;
    private static final int DW_TAG_shared_type = 0x40;

    private static final int DW_TAG_lo_user = 0x4080;
    private static final int DW_TAG_hi_user = 0xffff;

   /*
    *  ======== forms ========
    */
    private static final int DW_FORM_addr = 0x01; /* address */
    private static final int DW_FORM_block2 = 0x03; /* block */
    private static final int DW_FORM_block4 = 0x04; /* block */
    private static final int DW_FORM_data2 = 0x05; /* constant */
    private static final int DW_FORM_data4 = 0x06; /* constant, lineptr, loclistptr, macptr, rangelistptr */
    private static final int DW_FORM_data8 = 0x07; /* constant, lineptr, loclistptr, macptr, rangelistptr */
    private static final int DW_FORM_string = 0x08; /* string */
    private static final int DW_FORM_block = 0x09; /* block */
    private static final int DW_FORM_block1 = 0x0a; /* block */
    private static final int DW_FORM_data1 = 0x0b; /* constant */
    private static final int DW_FORM_flag = 0x0c; /* flag */
    private static final int DW_FORM_sdata = 0x0d; /* constant */
    private static final int DW_FORM_strp = 0x0e; /* string */
    private static final int DW_FORM_udata = 0x0f; /* constant */
    private static final int DW_FORM_ref_addr = 0x10; /* reference */
    private static final int DW_FORM_ref1 = 0x11; /* reference */
    private static final int DW_FORM_ref2 = 0x12; /* reference */
    private static final int DW_FORM_ref4 = 0x13; /* reference */
    private static final int DW_FORM_ref8 = 0x14; /* reference */
    private static final int DW_FORM_ref_udata = 0x15; /* reference */
    private static final int DW_FORM_indirect = 0x16 ;/* see Section 7.5.  */
    private static final int DW_FORM_sec_offset = 0x17; /* lineptr, loclistptr, macptr, rangelistptr */
    private static final int DW_FORM_exprloc = 0x18; /* exprloc */
    private static final int DW_FORM_flag_present = 0x19; /* flag */
    private static final int DW_FORM_ref_sig8 = 0x20; /* reference */

    /*
     *  ======== Form ========
     */
    private static class Form {
        public String name;
        public int size;

        public Form (String n, int sz) {
            this.name = n;
            this.size = sz;
        }
    }

    /*
     *  ======== forms ========
     *  Array of all DW_FORM* 
     */
    private static Form[] forms = {
        new Form("null", -1),
        new Form("DW_FORM_addr", -1),    /* 0x01,  address (size defined in CU header) */
        new Form("0x02", -1),            /* 0x02 */
        new Form("DW_FORM_block2", 2),   /* 0x03,  block */
        new Form("DW_FORM_block4", 4),   /* 0x04,  block */
        new Form("DW_FORM_data2", 2),    /* 0x05,  constant */
        new Form("DW_FORM_data4", 4),    /* 0x06,  constant, lineptr, loclistptr, macptr, rangelistptr */
        new Form("DW_FORM_data8", 8),    /* 0x07,  constant, lineptr, loclistptr, macptr, rangelistptr */
        new Form("DW_FORM_string", 0),   /* 0x08,  string ('\0' terminated chars) */
        new Form("DW_FORM_block", 0),    /* 0x09,  block (unsigned LEB128) */
        new Form("DW_FORM_block1", 1),   /* 0x0a,  block */
        new Form("DW_FORM_data1", 1),    /* 0x0b,  constant */
        new Form("DW_FORM_flag", 1),     /* 0x0c,  flag */
        new Form("DW_FORM_sdata", 0),    /* 0x0d,  constant (signed LEB128) */
        new Form("DW_FORM_strp", 4),     /* 0x0e,  string (8 for 64-bit dwarf) */
        new Form("DW_FORM_udata", 0),    /* 0x0f,  constant (unsigned LEB128) */
        new Form("DW_FORM_ref_addr", 4), /* 0x10,  reference (8 for 64-bit dwarf) */
        new Form("DW_FORM_ref1", 1),     /* 0x11,  reference */
        new Form("DW_FORM_ref2", 2),     /* 0x12,  reference */
        new Form("DW_FORM_ref4", 4),     /* 0x13,  reference */
        new Form("DW_FORM_ref8", 8),     /* 0x14,  reference */
        new Form("DW_FORM_ref_udata", 0),/* 0x15,  reference (unsigned LEB128) */
        new Form("DW_FORM_indirect", 0), /* 0x16,  see Section 7.5. (unsigned LEB128) */
        new Form("DW_FORM_sec_offset", 4), /* 0x17, lineptr, loclistptr, macptr, rangelistptr */
        new Form("DW_FORM_exprloc", 0),  /* 0x18,  exprloc */
        new Form("DW_FORM_flag_present", 0),  /* 0x19,  flag */
        new Form("0x1a", -1),            /* 0x1a */
        new Form("0x1b", -1),            /* 0x1b */
        new Form("0x1c", -1),            /* 0x1c */
        new Form("0x1d", -1),            /* 0x1d */
        new Form("0x1e", -1),            /* 0x1e */
        new Form("0x1f", -1),            /* 0x1f */
        new Form("DW_FORM_ref_sig8", 0), /* 0x20,  reference */
    };
 
    /*
     *  ======== tagNames ========
     *  Array of all DW_TAG*
     */
    private static String[] tagNames = {
        "null",                         /* 0x00 */
        "DW_TAG_array_type",            /* 0x01 */
        "DW_TAG_class_type",            /* 0x02 */
        "DW_TAG_entry_point",           /* 0x03 */
        "DW_TAG_enumeration_type",      /* 0x04 */
        "DW_TAG_formal_parameter",      /* 0x05 */
        "0x06",                         /* 0x06 */
        "0x07",                         /* 0x07 */
        "DW_TAG_imported_declaration",  /* 0x08 */
        "0x09",                         /* 0x09 */
        "DW_TAG_label",                 /* 0x0a */
        "DW_TAG_lexical_block",         /* 0x0b */
        "0x0c",                         /* 0x0c */
        "DW_TAG_member",                /* 0x0d */
        "0x0e",                         /* 0x0e */
        "DW_TAG_pointer_type",          /* 0x0f */
        "DW_TAG_reference_type",        /* 0x10 */
        "DW_TAG_compile_unit",          /* 0x11 */
        "DW_TAG_string_type",           /* 0x12 */
        "DW_TAG_structure_type",        /* 0x13 */
        "0x14",                         /* 0x14 */
        "DW_TAG_subroutine_type",       /* 0x15 */
        "DW_TAG_typedef",               /* 0x16 */
        "DW_TAG_union_type",            /* 0x17 */
        "DW_TAG_unspecified_parameters",/* 0x18 */
        "DW_TAG_variant",               /* 0x19 */
        "DW_TAG_common_block",          /* 0x1a */
        "DW_TAG_common_inclusion",      /* 0x1b */
        "DW_TAG_inheritance",           /* 0x1c */
        "DW_TAG_inlined_subroutine",    /* 0x1d */
        "DW_TAG_module",                /* 0x1e */
        "DW_TAG_ptr_to_member_type",    /* 0x1f */
        "DW_TAG_set_type",              /* 0x20 */
        "DW_TAG_subrange_type",         /* 0x21 */
        "DW_TAG_with_stmt",             /* 0x22 */
        "DW_TAG_access_declaration",    /* 0x23 */
        "DW_TAG_base_type",             /* 0x24 */
        "DW_TAG_catch_block",           /* 0x25 */
        "DW_TAG_const_type",            /* 0x26 */
        "DW_TAG_constant",              /* 0x27 */
        "DW_TAG_enumerator",            /* 0x28 */
        "DW_TAG_file_type",             /* 0x29 */
        "DW_TAG_friend",                /* 0x2a */
        "DW_TAG_namelist",              /* 0x2b */
        "DW_TAG_namelist_item",         /* 0x2c */
        "DW_TAG_packed_type",           /* 0x2d */
        "DW_TAG_subprogram",            /* 0x2e */
        "DW_TAG_template_type_parameter",  /* 0x2f */
        "DW_TAG_template_value_parameter", /* 0x30 */
        "DW_TAG_thrown_type",           /* 0x31 */
        "DW_TAG_try_block",             /* 0x32 */
        "DW_TAG_variant_part",          /* 0x33 */
        "DW_TAG_variable",              /* 0x34 */
        "DW_TAG_volatile_type",         /* 0x35 */
        "DW_TAG_dwarf_procedure",       /* 0x36 */
        "DW_TAG_restrict_type",         /* 0x37 */
        "DW_TAG_interface_type",        /* 0x38 */
        "DW_TAG_namespace",             /* 0x39 */
        "DW_TAG_imported_module",       /* 0x3a */
        "DW_TAG_unspecified_type",      /* 0x3b */
        "DW_TAG_partial_unit",          /* 0x3c */
        "DW_TAG_imported_unit",         /* 0x3d */
        "0x3e",                         /* 0x3e */
        "DW_TAG_condition",             /* 0x3f */
        "DW_TAG_shared_type",           /* 0x40 */
        "DW_TAG_type_unit",             /* 0x41 (Dwarf4) */
        "DW_TAG_rvalue_reference_type", /* 0x42 (Dwarf4) */
        "DW_TAG_template_alias",        /* 0x43 (Dwarf4) */
    };


    /*
     *  ======== attrNames ========
     *  Array of all DW_AT*
     */
    private static String[] attrNames = {
        "null",                     /* 0x00 */
        "DW_AT_sibling",            /* 0x01 (CU) reference */
        "DW_AT_location",           /* 0x02 */
        "DW_AT_name",               /* 0x03 (CU) string */
        "???",
        "???",
        "???",
        "???",
        "???",
        "DW_AT_ordering",           /* 0x09 */
        "???",
        "DW_AT_byte_size",          /* 0x0b */
        "DW_AT_bit_offset",         /* 0x0c */
        "DW_AT_bit_size",           /* 0x0d */
        "???",
        "???",
        "DW_AT_stmt_list",          /* 0x10 (CU) lineptr */
        "DW_AT_low_pc",             /* 0x11 (CU) address */
        "DW_AT_high_pc",            /* 0x12 (CU) address */
        "DW_AT_language",           /* 0x13 (CU) constant (LEB128) */
        "???",
        "DW_AT_discr",              /* 0x15 */
        "DW_AT_discr_value",        /* 0x16 */
        "DW_AT_visibility",         /* 0x17 */
        "DW_AT_import",             /* 0x18 */
        "DW_AT_string_length",      /* 0x19 */
        "DW_AT_common_reference",    /* 0x1a */
        "DW_AT_comp_dir",           /* 0x1b (CU) string */
        "DW_AT_const_value",        /* 0x1c */
        "DW_AT_containing_type",    /* 0x1d */
        "DW_AT_default_value",      /* 0x1e */
        "???",
        "DW_AT_inline",             /* 0x20 */
        "DW_AT_is_optional",        /* 0x21 */
        "DW_AT_lower_bound",        /* 0x22 */
        "???",
        "???",
        "DW_AT_producer",           /* 0x25 (CU) string */
        "???",
        "DW_AT_prototyped",         /* 0x27 */
        "???",
        "???",
        "DW_AT_return_addr",        /* 0x2a */
        "???",
        "DW_AT_start_scope",        /* 0x2c */
        "???",
        "DW_AT_bit_stride",         /* 0x2e */
        "DW_AT_upper_bound",        /* 0x2f */
        "???",
        "DW_AT_abstract_origin",    /* 0x31 */
        "DW_AT_accessibility",      /* 0x32 */
        "DW_AT_address_class",      /* 0x33 */
        "DW_AT_artificial",         /* 0x34 */
        "DW_AT_base_types",         /* 0x35 (CU) reference */
        "DW_AT_calling_convention", /* 0x36 */
        "DW_AT_count",              /* 0x37 */
        "DW_AT_data_member_location",/* 0x38 */
        "DW_AT_decl_column",        /* 0x39 */
        "DW_AT_decl_file",          /* 0x3a */
        "DW_AT_decl_line",          /* 0x3b */
        "DW_AT_declaration",        /* 0x3c */
        "DW_AT_discr_list",         /* 0x3d */
        "DW_AT_encoding",           /* 0x3e */
        "DW_AT_external",           /* 0x3f */
        "DW_AT_frame_base",         /* 0x40 */
        "DW_AT_friend",             /* 0x41 */
        "DW_AT_identifier_case",    /* 0x42 (CU) constant */
        "DW_AT_macro_info",         /* 0x43  macptr (CU) */
        "DW_AT_namelist_item",      /* 0x44  block */
        "DW_AT_priority",           /* 0x45  reference */
        "DW_AT_segment",            /* 0x46  block, loclistptr */
        "DW_AT_specification",      /* 0x47  reference */
        "DW_AT_static_link",        /* 0x48  block, loclistptr */
        "DW_AT_type",               /* 0x49  reference */
        "DW_AT_use_location",       /* 0x4a  block, loclistptr */
        "DW_AT_variable_parameter", /* 0x4b  flag */
        "DW_AT_virtuality",         /* 0x4c  constant */
        "DW_AT_vtable_elem_location",/* 0x4d  block, loclistptr */
        "DW_AT_allocated",          /* 0x4e  block, constant, reference */
        "DW_AT_associated",         /* 0x4f  block, constant, reference */
        "DW_AT_data_location",      /* 0x50  block */
        "DW_AT_byte_stride",        /* 0x51  block, constant, reference */
        "DW_AT_entry_pc",           /* 0x52  address */
        "DW_AT_use_UTF8",           /* 0x53  flag */
        "DW_AT_extension",          /* 0x54  reference */
        "DW_AT_ranges",             /* 0x55  rangelistptr */
        "DW_AT_trampoline",         /* 0x56  address, flag, reference, string */
        "DW_AT_call_column",        /* 0x57  constant */
        "DW_AT_call_file",          /* 0x58  constant */
        "DW_AT_call_line",          /* 0x59  constant */
        "DW_AT_description",        /* 0x5a  string */
        "DW_AT_binary_scale",       /* 0x5b  constant */
        "DW_AT_decimal_scale",      /* 0x5c  constant */
        "DW_AT_small",              /* 0x5d  reference */
        "DW_AT_decimal_sign",       /* 0x5e  constant */
        "DW_AT_digit_count",        /* 0x5f  constant */
        "DW_AT_picture_string",     /* 0x60  string */
        "DW_AT_mutable",            /* 0x61  flag */
        "DW_AT_threads_scaled",     /* 0x62  flag */
        "DW_AT_explicit",           /* 0x63  flag */
        "DW_AT_object_pointer",     /* 0x64  reference */
        "DW_AT_endianity",          /* 0x65  constant */
        "DW_AT_elemental",          /* 0x66  flag */
        "DW_AT_pure",               /* 0x67  flag */
        "DW_AT_recursive",          /* 0x68  flag */
        "DW_AT_signature",          /* 0x69  reference (Dwarf4) */
        "DW_AT_main_subprogram",    /* 0x6a  flag (Dwarf4) */
        "DW_AT_data_bit_offset",    /* 0x6b  constant (Dwarf4) */
        "DW_AT_const_expr",         /* 0x6c  flag (Dwarf4) */
        "DW_AT_enum_class",         /* 0x6d  flag (Dwarf4) */
        "DW_AT_linkage_name",       /* 0x6e string (Dwarf4) */
    };

    /*
     *  ======== abbrevRecArr ========
     *  Abbreviation Records are read once into this array
     */
    private Dwarf32.Abbrev[] abbrevRecArr = null;

    /*
     *  ======== Types ========
     *  Class spec to hold Type information
     *
     *  If the type is a structure type, 'subtypes' contains
     *  <offset, subtype id> pairs for each member of the structure. Otherwise,
     *  'subtypes' is null.
     */
    private class Types
    {
        public String name;   /* Name of the type */
        public int type;      /* Address to base type (if any) */
        public HashMap<Integer, Integer> subtypes;
        public int elnum;     /* Number of elements for array types */
        public int elsize;    /* Element size for array types */
    }

    /*
     *  ======== globalMap ========
     *  HashMap of global variables
     *
     *  Stores name of the variables and its type address
     */
    private HashMap<String, Integer> globalMap = null;

    /*
     *  ======== typesMap ========
     *  HashMap of all data type information
     *
     *  Stores type address and if it is not the base type then the address
     *  of its type.
     */
    private HashMap<Integer, Types> typesMap = null;

    /*
     *  ======== Dwarf32 ========
     *  Constructor
     */
    public Dwarf32()
    {
        globalMap = new HashMap<String, Integer>();
        typesMap  = new HashMap<Integer, Types>();
        lastAbbrevFound = 0;
    }

    /*
     *  ======== Variable ========
     *  Class spec to hold Variable information
     */
    public class Variable
    {
        public String name;    /* Variable Name, if there is a '.' in the name
                                * 'type' refers to a structure member at the
                                * specified offset.
                                */
        public String type;    /* Base type of the variable, or of the member
                                * at the specified offset.
                                */
        public int offset;
    }

    /*
     *  ======== CUHeader ========
     *  Class spec to hold Compilation Unit Header information
     */
    private class CUHeader
    {
        public int   unit_length;          /* length of compilation unit */
        public short version;
        public int   debug_abbrev_offset;  /* offset into abbrev rec section */
        public byte  address_size;

        public void readCUHeader(ByteBuffer buf) {
           unit_length = buf.getInt();
           version = buf.getShort();
           debug_abbrev_offset = buf.getInt();
           address_size = buf.get();
           forms[DW_FORM_addr].size = address_size;
        }
    }

    /*
     *  ======== DieAttr ========
     *  Class spec to hold DIE attribute information
     */
    private class DieAttr
    {
        public int id;
        public int form;
    }

    /*
     *  ======== Abbrev ========
     *  Class spec to hold Abbreviation record information
     */
    private class Abbrev {
        public int             offset;    /* offset of DIE in the abbrev rec */
        public int             code;      /* info that links various recs */
        public int             tag;       /* DIE type */
        public boolean         children;  /* Does this DIE have children? */
        public Dwarf32.DieAttr[] attrs;   /* variable num of Dwarf32_DieAttr */

        public Abbrev() {
            offset = 0;
            code = 0;
            tag =  0;
            children = false;
            attrs = null;
        }
    }

    private int lastAbbrevFound;

    /*
     *  ======== getAbbrevRec ========
     *  Get the abbreviation record 
     *
     *  Get the abbreviation for the given abbreviation code starting at the
     *  abbreviation offset.
     *
     *  Note: This function is optimized using the assumption that the supplied
     *  abbreviation offset is always the same or just a little larger than the
     *  offset supplied in the previous call. The index where the previous
     *  offset was found is kept in 'lastAbbrevFound' and that's the index where
     *  the search starts in the current call.
     */
    private Dwarf32.Abbrev getAbbrevRec(int abbrevOffset, int code)
        throws Exception
    {
        Dwarf32.Abbrev abbrevRec = null;

        if (abbrevRecArr == null) {
            throw new Exception("Abbreviation record array is null");
        }

        int i = lastAbbrevFound;
        do {
            abbrevRec = abbrevRecArr[i];
            /* Skip till the offset */
            if (abbrevRec.offset == abbrevOffset) {
                lastAbbrevFound = i;
                for (int j = i; j < abbrevRecArr.length; j++) {
                    abbrevRec = abbrevRecArr[j];
                    /* Now let's keep checking the code */
                    if (abbrevRec.code == code) {
                        break;
                    }
                }
                break;
            }
            i = (i < abbrevRecArr.length - 1) ? i + 1 : 0;
        } while (i != lastAbbrevFound);

        if (abbrevRec == null || abbrevRec.code != code) {
            String errorMsg = "Failed to find abbrev code " + code +
                               " from offset " + abbrevOffset; 
            throw new Exception(errorMsg);
        }
        return (abbrevRec);
    }

    /*
     *  ======== getAttrName ========
     *  Get the DW_AT_* string
     */
    private String getAttrName(int attr)
    {
        if (attr < attrNames.length) {
            return (attrNames[attr]);
        }
        else if (attr >= DW_AT_lo_user && attr <= DW_AT_hi_user) {
            return ("DW_AT_user_" + Integer.toHexString(attr));
        }
        else {
            return ("<unknown DW_AT value: " + Integer.toHexString(attr) + ">");
        }

    }

    /*
     *  ======== getBaseType ========
     *  Recursively navigate to get the base type name
     *
     *  This function follows the chain of typedefs, or a relation between an
     *  array and its element type, or between a pointer type and its base type.
     *  In most cases we are looking for Mod_Struct types, and they are at the
     *  end of such chains. But, if we started looking for other types, the
     *  logic that uses this function might not work.
     */
    private Types getBaseType(HashMap<Integer, Types> typesMap, int type)
    {
        Types currType, newType;
        if ((currType = typesMap.get(type)) == null) {
            return (null);
        }

        if ((newType = getBaseType(typesMap, currType.type)) == null) {
            return (currType);
        }
        else {
            return (newType);
        }
    }

    /*
     *  ======== getGlobalVariables ========
     *  Get all the global variables and their type information
     */
    public Variable[] getGlobalVariables() throws Exception
    {
         return (this.getGlobalVariablesByType(".*"));
    }

    /*
     *  ======== getGlobalVariablesByType ========
     *  Get global variables of particular types matching the regex
     *
     *  @params(typeRegEx)  Regular expression to limit types of variables
     */
    public Variable[] getGlobalVariablesByType(String typeRegEx)
        throws Exception
    {
        ArrayList<Variable> varList = new ArrayList<Variable>();

        /* Populate an array of Variables with name and type info */
        for (Map.Entry<String, Integer> entry: globalMap.entrySet()) {
            String varName = entry.getKey();
            /* if this entry is an array variable, directType will have elnum
             * larger than 0,
             */
            Types directType = typesMap.get(entry.getValue());
            /* directType should normally never be null, but we don't extract
             * information about each type to typesMap. Arrays are, for example,
             * ignored currently.
             */

            if (directType == null) {
                continue;
            }
            Types typeObj = this.getBaseType(typesMap, entry.getValue());

            /* typeObj is never 'null' because we checked for that case
             * through directType.
             */
            String varType = typeObj.name;

            int counter = 1;
            int offset = 0;
            if (directType != null && directType.elnum > 0) {
                counter = directType.elnum;
                offset = directType.elsize;
                if (counter > 1 && offset == 0 && typeObj.elsize > 0) {
                    /* It's an array but we don't know an element's size.
                     * We'll look into the basic definition of a type. We only
                     * care about aggregate types and they are the ones that
                     * might have size recorded.
                     */
                    offset = typeObj.elsize;
                }
            }

            /* This loop goes through globals and checks if any of them, or any
             * member of a global structure matches the regular expression.
             */
            for (int i = 0; i < counter; i++) {
                String nm = varName;
                if (offset > 0) {
                    nm = varName + "." + i;
                }
                getVariableByType(typeRegEx, typeObj, nm, i * offset, varList,
                                  0);
            }
        }
        return (varList.toArray(new Variable[varList.size()]));
    }

    /*
     *  ======== getVariableByType ========
     *  Recursive function that goes through types of structure members and
     *  detects if any of them matches the regular expression. The type of the
     *  member and its offset within a structure are captured. The name under
     *  which this new variable is saved is the name of the global variable to
     *  which the current offset is added. This makes each name unique and when
     *  the variables are saved in a hash map in xdc.rov, theer won't be any
     *  issue of one variable overwriting another. It's important that a global
     *  variable name is separated by '.' from the rest of the name. The code in
     *  xdc.rov will look for '.' to detect the name to use for a symbol table
     *  lookup.
     *
     *  @params(typeRegEx)      Regular expression for types
     *  @params(typeObj)        base type of the current variable
     *  @params(varName)        name of the current variable
     *  @params(currentOffset)  offset from the address of the global variable
     *  @params(varList)        list of variables that macthed typeRegEx
     *  @params(depth)          how far into embedded structures we go
     */
    private void getVariableByType(String typeRegEx, Types typeObj,
        String varName, int currentOffset, ArrayList<Variable> varList,
        int depth)
    {
        if (depth >= 3) return;
        if (typeObj.name.matches(typeRegEx)) {
            Variable var = new Variable();
            var.name = varName;
            var.type = typeObj.name;
            var.offset = currentOffset;
            varList.add(var);
        }
        else if (typeObj.subtypes != null) {

            for (Map.Entry<Integer, Integer> st: typeObj.subtypes.entrySet()) {
                Types subType = this.getBaseType(typesMap, st.getValue());
                if (subType == null) {
                    return;
                }
                String nm = varName;
                Pattern p = Pattern.compile(".+_(\\w*)_Struct");
                Matcher m = p.matcher(subType.name);
                int nextOffset = currentOffset + st.getKey();
                if (m.matches() && m.group(1) != null) {
                    nm = nm + "." + m.group(1) + "_" + nextOffset;
                }
                getVariableByType(typeRegEx, subType, nm, nextOffset, varList,
                    depth + 1);
            }
        }
    }

    /*
     *  ======== getTagName ========
     *  Get the DW_TAG_* string
     */
    private String getTagName(int tag)
    {
        if (tag < tagNames.length) {
            return (tagNames[tag]);
        }
        else if (tag >= DW_TAG_lo_user && tag <= DW_TAG_hi_user) {
            return ("DW_TAG_user_"+ Integer.toHexString(tag));
        }
        else {
            return ("<unknown DW_TAG value: " + Integer.toHexString(tag) + ">");
        }
    }

    /*
     *  ======== parse ========
     *  Parse dwarf information to get global variables info
     *
     *  Parses the .debug_info, .debug_abbrev and .debug_str sections of
     *  DWARF data to populate a list of variables and their types.
     *
     *  @params(info)   byte buffer containing .debug_info section.
     *  @params(abbrev) byte buffer containing .debug_abbrev section.
     *  @params(str)    byte buffer containing .debug_str section. Certain code
     *                  gen tools do not support this section. So this can be
     *                  null.
     */
    public void parse(ByteBuffer info, ByteBuffer abbrev, ByteBuffer str)
        throws Exception
    {
        if (info == null) {
            throw new Exception("Cannot read .debug_info");
        }

        if (abbrev == null) {
            throw new Exception("Cannot read .debug_abbrev");
        }

        /* read abbrevation record into a buffer */
        readAbbrevRecArr(abbrev);

        /* Decode the .debug_info section */
        while (info.remaining() > 0) {
            CUHeader header = new CUHeader();
            int abbrevCode = 0;
            Dwarf32.Abbrev abbrevRec = null;

            /* Compilation unit tracker */
            int cuOffset = info.position();

            /* Get compilation unit information from the header */
            header.readCUHeader(info);

            /* get abbreviation code (record offset) */
            abbrevCode = this.readULEB128(info);

            /* Get the compilation unit DIE abbreviation record */
            abbrevRec = this.getAbbrevRec(header.debug_abbrev_offset,
                                          abbrevCode);

            /* loop through CU DIE. We don't need this info */
            for (int k = 0; k < abbrevRec.attrs.length; k++) {
                int id = abbrevRec.attrs[k].id;
                int form = abbrevRec.attrs[k].form;
                int size = forms[form].size;

                if (size > 0) {
                    if ((id == DW_AT_comp_dir) || (id == DW_AT_name)
                        || (id == DW_AT_stmt_list)) {
                        info.getInt();
                    }
                    else {
                        info.position(info.position() + size);
                    }
                }
                else if (size == 0) {
                    if (form == DW_FORM_string) {
                        this.readStringFromBuffer(info, info.position());
                    }
                    else {
                        this.readULEB128(info);
                    }
                }
                else {
                    throw new Exception("Size cannot be < 0 : " + size);
                }
            }

            /* Looping through all the DIEs */
            int level = 1; /* Keep track of the symbol scope */

            /* When a structure is found, the DIEs that follow are DW_TAG_member
             * DIEs. Until we detect one that's not DW_TAG_member, we record the
             * type ID and the offset within a structure for each member. This
             * flag is true while member DIEs are being processed.
             */
            boolean structMemberActive = false;
            /* After an array type, there is another DIE, which specifies the
             * number of elements in DW_AT_upper_bound. This flag is true while
             * that DIE is being processed.
             */
            boolean arrayTypeActive = false;

            /* The following objects and variables must keep values over more
             * than one DIE while arrayTypeActive or structMemberActive is true.
             */
            Types aggregateType = null;
            int aggregateOffset = 0;
            int structSize = 0;
            int arraySize = 0;
            int arrayLen = 0;
            for (int j = 0;
                 info.position() < (cuOffset + header.unit_length + 4); j++) {
                /* Keep track of DIE position */
                int dieOffset = info.position();

                /* flags for info extraction */
                boolean isVar = false;
                boolean isType = false;
                boolean isStructureType = false;
                boolean isArrayType = false;

                /* temporary storage */
                String atName = "";
                String atType = "0";
                int memberOffset = 0;
                Types typeObj = null;

                abbrevCode = this.readULEB128(info);
                /* if it is a null DIE, skip it */
                if (abbrevCode == 0) {
                    if (level > 1 ) {
                        level--;
                    }
                    continue;
                }

                abbrevRec = this.getAbbrevRec(header.debug_abbrev_offset,
                                              abbrevCode);

                /* Is it either a variable or type DIE? */
                String tagName = getTagName(abbrevRec.tag);
                if (structMemberActive && !(tagName.matches("DW_TAG_member"))) {
                    /* no more members in a structure, it's time to add the
                     * structure type to the list.
                     */
                    structMemberActive = false;
                    typesMap.put(aggregateOffset, aggregateType);
                }

                if (arrayTypeActive
                    && !(tagName.matches("DW_TAG_subrange_type"))) {
                    arrayTypeActive = false;
                    if (arrayLen != 0) {
                        typesMap.put(aggregateOffset, aggregateType);
                    }
                }

                if (tagName.matches("DW_TAG_variable")) {
                    isVar = true;
                }
                else if (tagName.matches(".*structure_type")) {
                    isStructureType = true;
                    aggregateType = new Types();
                    aggregateOffset = dieOffset;
                    structSize = 0;
                }
                else if (tagName.matches(".*array_type")) {
                    isArrayType = true;
                    aggregateType = new Types();
                    aggregateOffset = dieOffset;
                    arraySize = 0;
                    arrayLen = 0;
                }
                else if (tagName.matches(".*type.*")) {
                    isType = true;
                    typeObj = new Types();
                }

                /* Loop through DIE attributes */
                for (int k = 0; k < abbrevRec.attrs.length; k++) {
                    int form = abbrevRec.attrs[k].form;
                    int size = forms[form].size;
                    String name = getAttrName(abbrevRec.attrs[k].id);
                    String value = "";
                    /* There are cases when we know we are reading an integer,
                     * and that the value will be copied to another integer.
                     * So, instead of converting to and from 'value', we use
                     * 'intValue'.
                     */
                    int intValue = 0;

                    if (size > 0) {
                        if (form == DW_FORM_strp) {
                            if (str == null) {
                                throw new Exception("DW_FORM_strp shouldn't appear with null string table");
                            }
                            value = this.readStringFromBuffer(str, info.getInt());
                        }
                        else if (form == DW_FORM_ref1) {
                            value = Integer.toString(cuOffset + info.get());
                        }
                        else if (form == DW_FORM_ref2) {
                            value = Integer.toString(cuOffset
                                + info.getShort());
                        }
                        else if (form == DW_FORM_ref4) {
                            value = Integer.toString(cuOffset + info.getInt());
                        }
                        else if (form == DW_FORM_ref8
                            || form == DW_FORM_ref_sig8) {
                            value = Long.toString(cuOffset + info.getLong());
                        }
                        else if (form == DW_FORM_ref_addr
                            || form == DW_FORM_sec_offset) {
                            value = Integer.toString(info.getInt());
                        }
                        else {
                            int bsize = 0;
                            switch (size) {
                                case 1: {
                                    bsize = info.get();
                                    break;
                                }
                                case 2: {
                                    bsize = info.getShort();
                                    break;
                                }
                                case 4: {
                                    bsize = info.getInt();
                                    break;
                                }
                                /* bsize should be long?
                                case 8: {
                                    bsize = info.getLong();
                                    break;
                                }
                                */
                                default: {
                                    /* Should not happen? */
                                    throw new Exception(name + "has an unsupported size: " +  size);
                                }
                            }
                            /* DW_FORM_data1 values can be a signed or an
                             * unsigned integer. We are here only interested in
                             * array lengths or array sizes so we will assume
                             * that they were originally unsigned, but Java
                             * turned them into signed integers.
                             */
                            if (bsize < 0) {
                                bsize = bsize & (~(0xffffffff << (8 * size)));
                            }
                            /* If we are not processing blockN code below, value
                             * will contain an actual attribute value.
                             */
                            value = Integer.toString(bsize);

                            if (form == DW_FORM_data1 || form == DW_FORM_data2)
                            {
                                intValue = bsize;
                            }
                            /* Blocks contain the operations. We are interested
                             * in them because member offsets in structures are
                             * sometimes encoded that way.
                             * The first value that we read is an operation
                             * code. The code '0x23' means that the operation is
                             * DW_OP_plus_uconst, with only one operand in
                             * ULEB128 format. Other operations could be used
                             * here and we should add them eventually.
                             */
                            else if (form == DW_FORM_block1
                                || form == DW_FORM_block2
                                || form == DW_FORM_block4) {
                                bsize--;
                                value = "";
                                if (info.get() == 35) {
                                    intValue = readULEB128(info);
                                }
                                else {
                                    info.position(info.position() + bsize);
                                }
                            }
                        }
                    }
                    else if (size == 0) {
                        switch (form) {
                            case DW_FORM_flag_present: {
                                /* abbrev record specifies that the attribute
                                 * is present, but there is no value encoded in
                                 * DIE.
                                 */
                                break;
                            }
                            case DW_FORM_string: {
                                value = this.readStringFromBuffer(info,
                                        info.position());
                                break;
                            }
                            case DW_FORM_block:
                            case DW_FORM_exprloc: {
                                int bsize = readULEB128(info);
                                bsize--;
                                value = ""; /* just to be sure */
                                if (info.get() == 35) {
                                    intValue = readULEB128(info);
                                }
                                else {
                                    while (bsize != 0) {
                                        /* equivalent to integer shifting */
                                        value += Integer.toString(info.get());
                                        bsize--;
                                    }
                                }
                                break;
                            }
                            case DW_FORM_sdata: {
                                int formVal = readSLEB128(info);
                                value = Integer.toString(formVal);
                                break;
                            }
                            default: {
                                int formVal = readULEB128(info);
                                if (form == DW_FORM_udata) {
                                    value = Integer.toString(formVal);
                                }
                                else {
                                    value = Integer.toString(cuOffset + formVal);
                                }
                                break;
                            }
                        }
                    }
                    else {
                        /* Should not happen! */
                        throw new Exception(name + " has an  invalid form: "
                            + form);
                    }

                    /* Save name and type info in temporary variables */
                    if (name == "DW_AT_name") {
                        atName = value;
                    }
                    else if (name == "DW_AT_type") {
                        atType = value;
                    }

                    if (arrayTypeActive && name == "DW_AT_upper_bound") {
                        arrayLen = Integer.parseInt(value) + 1;
                    }
                    else if (structMemberActive
                        && name == "DW_AT_data_member_location") {
                        memberOffset = intValue;
                    }
                    else if (isArrayType && name == "DW_AT_byte_size") {
                        arraySize = Integer.parseInt(value);
                    }
                    else if (isStructureType && name == "DW_AT_byte_size") {
                        structSize = Integer.parseInt(value);
                    }

                } /* finished looping through attributes for a DIE */
                if (abbrevRec.children) {
                    level++;
                }

                /* Store variables if it is global(based on level) */
                if (isVar && (level == 1) && atName != "") {
                    globalMap.put(atName, Integer.parseInt(atType));
                }

                if (atType == "") {
                    atType = "0";
                }

                if (structMemberActive) {
                    if (Integer.parseInt(atType) != 0) {
                        aggregateType.subtypes.put(memberOffset,
                                            Integer.parseInt(atType));
                    }
                }
                else if (arrayTypeActive) {
                    aggregateType.elnum = arrayLen;
                    if (arrayLen != 0) {
                        aggregateType.elsize = arraySize / arrayLen;
                    }
                }

                else if (isType) {
                    typeObj.name = atName;
                    typeObj.type = Integer.parseInt(atType);
                    typeObj.subtypes = null;
                    typeObj.elnum = 0;
                    typeObj.elsize = 0;
                    typesMap.put(dieOffset, typeObj);
                }
                else if (isArrayType) {
                    aggregateType.name = atName;
                    aggregateType.type = Integer.parseInt(atType);
                    aggregateType.elsize = arraySize;
                    aggregateType.subtypes = null;
                    aggregateType.elnum = 0;
                    arrayTypeActive = true;
                }
                else if (isStructureType) {
                    aggregateType.name = atName;
                    aggregateType.type = Integer.parseInt(atType);
                    aggregateType.subtypes = new HashMap<Integer, Integer>();
                    aggregateType.elsize = structSize;
                    aggregateType.elnum = 0;
                    structMemberActive = true;
                }
            }
        }
        //printTypes();
    }

    /*
     *  ======== readAbbrevRecArr ========
     *  Parse the abbreviation record information
     *
     *  The ByteBuffer containing abbreviation records section is parsed
     *  and put in a global array to be accessed later.
     */
    private void readAbbrevRecArr(ByteBuffer abbrev) throws Exception
    {
        ArrayList<Dwarf32.Abbrev> arecList = new ArrayList<Dwarf32.Abbrev>();

        while (abbrev.remaining() > 0) {
            Dwarf32.Abbrev arec = new Dwarf32.Abbrev();
            ArrayList<Dwarf32.DieAttr> dieAttrList = new ArrayList<Dwarf32.DieAttr>();

            arec.offset = abbrev.position();
            arec.code = this.readULEB128(abbrev);

            if (arec.code != 0 && abbrev.remaining() > 0) {
                arec.tag = this.readULEB128(abbrev);
                arec.children = abbrev.get() == 0 ? false : true;

                while (abbrev.remaining() > 0) {
                    Dwarf32.DieAttr dieAttr = new Dwarf32.DieAttr();
                    dieAttr.id = this.readULEB128(abbrev);
                    dieAttr.form = this.readULEB128(abbrev);

                    /* dieAttr ends with a zeroed id and form */
                    if (dieAttr.id == 0 && dieAttr.form == 0) {
                        break;
                    }
                    dieAttrList.add(dieAttr);

                }
                arec.attrs = dieAttrList.toArray(new Dwarf32.DieAttr[dieAttrList.size()]);
            }
            arecList.add(arec);
        }

        /* Store it away in global array */
        abbrevRecArr = (arecList.toArray(new Dwarf32.Abbrev[arecList.size()]));
    }

    /*
     *  ======== readStringFromBuffer ========
     */
    private String readStringFromBuffer(ByteBuffer buffer, int offset)
    {
        /* Move the ByteBuffer's position to the specified offset. */
        buffer.position(offset);

        /* Walk through the buffer until we find the null terminator. */
        while (buffer.get() != 0) {
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

        /* remove null char */
        buffer.get();

        /* Create a String from the buffer and return it. */
        return (new String(stringBuf, 0, strLen));
    }

    /*
     *  ======== readSLEB128 ========
     *  Read Signed Little Endian Base 128 data
     *
     *  Note: Read DWARF Spec for more info on LEB128
     */
    private int readSLEB128(ByteBuffer buf)
    {
        byte byt = 0;
        int  result = 0;
        int  shift = 0;
        int  size = 32;       /* number of bits */

        /* 32 bit can be spread across 5 bytes: 4 7 7 7 7 bits */
        for (int i = 0; i < 5; i++, shift +=7) {
            byt = buf.get();
            result |= (0x7f & byt) << shift;
            if ((0x80 & byt) == 0) {
                break;
            }
        }
        /* sign padding */
        if ((shift < 32) && (0x40 & byt) == 1) {
            result |= - (1 << shift);
        }

        return (result);
    }

    /*
     *  ======== readULEB128 ========
     *  Read Unsigned Little Endian Base 128 data
     *
     *  Note: Read DWARF Spec for more info on LEB128
     */
    private int readULEB128(ByteBuffer buf)
    {
        byte byt = 0;
        int  result = 0;
        int  shift = 0;

        /* 32 bit can be spread across 5 bytes: 4 7 7 7 7 bits */
        for (int i = 0; i < 5; i++, shift +=7) {
            byt = buf.get();
            result |= (0x7f & byt) << shift;
            if ((0x80 & byt) == 0) {
                break;
            }
        }

        return (result);
    }

    /*
     *  ======== printVariables ========
     *  Prints all the variables from the array
     */
    static public void printVariables(Variable vars[])
    {
        for (int varIndex = 0; varIndex < vars.length; varIndex++) {
            System.out.println("(" + (varIndex + 1) + ")\n"
                              + "    name: " + vars[varIndex].name + "\n"
                              + "    type: " + vars[varIndex].type + "\n"
                              + "  offset: " + vars[varIndex].offset + "\n");
        }
    }

    public void printTypes()
    {
        for (Map.Entry<Integer, Types> entry: typesMap.entrySet()) {
            Integer typeInt = entry.getKey();
            String name = entry.getValue().name;
            int typeLittleInt = entry.getValue().type;
            System.out.println("Type Name: " + name);
            System.out.println("Values: " + typeInt + "  " + typeLittleInt);
            if (entry.getValue().subtypes == null) continue;
            for (Map.Entry<Integer, Integer> subt:
                entry.getValue().subtypes.entrySet()) {
                int subtype = subt.getKey();
                int offset = subt.getValue();
                System.out.println("{" + subtype + ", " + offset + "}");
            }
        }

    }

    public void printVariables()
    {
        for (Map.Entry<String, Integer> entry: globalMap.entrySet()) {
            String name = entry.getKey();
            Integer type = entry.getValue();
            System.out.println("Var Name: " + name);
            System.out.println("Value: " + type);
        }
    }

    /*
     *  ======== main ========
     *  Unit test for this class
     *
     *  Note: The below code is commented to remove dependency on 
     *        ELF. For unit testing, this code can be uncommented.
     */
    public static void main(String args[]) throws Exception
    {
        if ((args.length != 1)) {
            System.out.println("Usage: dwarf elffile\n"
                             + "       dwarf a.out");
            return;
        }

        Elf32 elf = new Elf32();
        elf.parse(args[0]);
        Dwarf32.Variable[] vars = elf.getGlobalVariablesByType(".*_Struct");
        Dwarf32.printVariables(vars);
    }
}
