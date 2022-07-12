#include "dns_ropob.h"

void initialize_main(int func_count, FILE* outasm_fp) {
	char buf[1001];
	asm_write("\tpushfq\n", outasm_fp);
	asm_write("\tpush\trax\n", outasm_fp);
	asm_write("\tpush\trcx\n", outasm_fp);
	asm_write("\tpush\trdx\n", outasm_fp);
	asm_write("\tlea\trax, funcgadgetoffsets[rip]\n", outasm_fp);
	for (int i = 0; i < func_count; i++) {
		sprintf(buf, "\tlea\trcx, func%dgadgettable[rip]\n", i);
		asm_write(buf, outasm_fp);
		sprintf(buf, "\tmov\trdx, %d\n", i);
		asm_write(buf, outasm_fp);
		asm_write("\tmov\tQWORD PTR [rax+rdx*8], rcx\n", outasm_fp);
	}
	asm_write("\tpop\trdx\n", outasm_fp);
	asm_write("\tpop\trcx\n", outasm_fp);
	asm_write("\tpop\trax\n", outasm_fp);
	asm_write("\tpopfq\n", outasm_fp);
	asm_write("\tnop\n", outasm_fp);
	asm_write("\tnop\n", outasm_fp);
}

void write_gadget(int func_number, int gadget_number, char* funcname, char *gadget_string, FILE *outasm_fp) {
	char buf[1001];
	asm_write(gadget_string, outasm_fp);
	asm_write("\tpushfq\n", outasm_fp);
	asm_write("\tpushfq\n", outasm_fp);
	asm_write("\tpush\trax\n", outasm_fp);
	sprintf(buf, "\tlea\trax, %s[rip]\n", funcname);
	asm_write(buf, outasm_fp);
	asm_write("\tmov\tQWORD PTR base[rip], rax\n", outasm_fp);
	sprintf(buf, "\tmov\tQWORD PTR gadgetnumber[rip], %d\n", gadget_number);
	asm_write(buf, outasm_fp);
	sprintf(buf, "\tmov\tQWORD PTR funcnumber[rip], %d\n", func_number);
	asm_write(buf, outasm_fp);
	asm_write("\tlea\trax, resolver[rip]\n", outasm_fp);
	asm_write("\tmov\tQWORD PTR [rsp+16], rax\n", outasm_fp);
	asm_write("\tpop\trax\n", outasm_fp);
	asm_write("\tpopfq\n", outasm_fp);
	asm_write("\tret\n", outasm_fp);
	return;
}

void copy_asm_to_outasm(FILE* asm_fp, FILE* outasm_fp) {
	char buf[1001];
	// asm_fpの位置を調整
	rewind(asm_fp);
	for (int i = 0;i < 3; i++){
		fgets(buf, 1000, asm_fp);
	}
	char* funcname;
	char funclabel[101];
	for (int i = 0; i < func_count; i++) {
		int func_number = i;
		int instruction_count = *function_table[func_number][3];
		funcname = funcname_table[func_number];
		strncpy(funclabel, funcname, 100);
		strncat(funclabel, ":", 1);
		while(strstr(buf, funclabel) == NULL){
			fgets(buf, 1000, asm_fp);
			fwrite(buf, sizeof(char), strlen(buf), outasm_fp);
		}
		if(strncmp(funcname, "main\0", 5) == 0) {
			initialize_main(func_count, outasm_fp);
		}
		char* gadget_string[instruction_count];
		for (int j = 0; j < instruction_count; j++) {
			fgets(buf, 1000, asm_fp);
			if(buf[0] != '\t') {
				char label[1001];
				strncpy(label, buf, 1001);
				fgets(buf, 1000, asm_fp);
				strncat(label, buf, 1001);
				gadget_string[j] = malloc(strlen(label) + 1);
				strncpy(gadget_string[j], label, strlen(label) + 1);
			} else {
				gadget_string[j] = malloc(strlen(buf)+1);
				strncpy(gadget_string[j], buf, strlen(buf) + 1);
			}
		}
		for (int j = 0; j < instruction_count; j++) {
			int gadget_number = function_table[func_number][0][j];
			write_gadget(func_number, gadget_number, funcname ,gadget_string[gadget_number], outasm_fp);
		}
	}
	while(fgets(buf, 1000, asm_fp) != NULL) {
		fwrite(buf, sizeof(char), strlen(buf), outasm_fp);
	}
	fwrite("\n", sizeof(char), 1, outasm_fp);
	return;
}
