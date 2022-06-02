#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// global pointer
FILE *asm_fp;
FILE *outasm1_fp;
FILE *obj1_fp;
FILE *outasm2_fp;
FILE *obj2_fp;


// global table
char** funcname_table;
int func_count;
int*** function_table;


// dns_ropob.c
void error(char* error_msg, int error_number);

// init_function_info.c
void make_global(char* globalname, FILE *outasm_fp);
void init_function_info(FILE *asm_fp, FILE *outasm_fp, FILE *obj_fp);

// make_function_gadget_table.c
void make_func_offs_table(int offsets, FILE *outasm_fp);
void make_function_gadget_table(int funcnumber, int instruction_count, char *funcname, FILE *outasm_fp, FILE *obj_fp);
void make_function_gadget_table_fix_jmp(FILE* outasm1_fp, FILE* outasm2_fp, FILE* obj2_fp, int funcnumber);


// add_resolver.c
void asm_write(char *str, FILE *outasm_fp);
void add_resolver(FILE *outasm_fp);

// copy_asm_to_outasm.c
void copy_asm_to_outasm(FILE *asm_fp, FILE *outasm_fp);

// copy_outasm1_to_outasm2.c
void copy_outasm1_to_outasm2(FILE *outasm1_fp, FILE *outasm2_fp, FILE *obj2_fp);
