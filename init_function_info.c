#include "dns_ropob.h"

void make_global(char* globalname, FILE *outasm_fp) {
	char asm[1000];
	sprintf(asm, "\t.globl\t%s\n\t.data\n\t.align\t8\n\t.type\t%s, @object\n\t.size\t%s, 8\n%s:\n\t.quad\t0\n", globalname, globalname, globalname, globalname);
	fwrite(asm, sizeof(char), strlen(asm), outasm_fp);
	return;
}

void make_function_gadget_offsets(int funcnumber, int instruction_count,int func_count, char* funcname, FILE *outasm_fp, FILE *obj_fp) {
	int gadget_table_size = instruction_count * 8;
	if (funcnumber == 0) {
		char asm[100] = "\t.globl\tfuncgadgetoffsets\n\t.align\t8\n\t.type\tfuncgadgetoffsets, @object\n";
		fwrite(asm, sizeof(char), strlen(asm), outasm_fp);
		sprintf(asm, "\t.size\tfuncgadgetoffsets, %d\nfuncgadgetoffsets:\n", 8*func_count);
		fwrite(asm, sizeof(char), strlen(asm), outasm_fp);
		for(int i = 0; i < func_count; i++) {
			fwrite("\t.quad\t0\n", sizeof(char), 9, outasm_fp);
		}
	}
	char asm[1000];
	sprintf(asm, "\t.globl\tfunc%dgadgettable\n\t.align\t8\n\t.type\tfunc%dgadgettable, @object\n", funcnumber, funcnumber);
	fwrite(asm, sizeof(char), strlen(asm), outasm_fp);
	sprintf(asm, "\t.size\tfunc%dgadgettable, %d\nfunc%dgadgettable:\n", funcnumber, gadget_table_size, funcnumber);
	fwrite(asm, sizeof(char), strlen(asm), outasm_fp);
	make_function_gadget_table(funcnumber, instruction_count, funcname, outasm_fp, obj_fp);
	return;
}

void init_function_info(FILE *asm_fp, FILE *outasm_fp, FILE *obj_fp) {
	char buf[1001];
	func_count = 0;
	int funcnumber = 0;
	int infunc = 0;
	int instruction_count = 0;
	rewind(asm_fp);

	// functionの個数を確認
	while(fgets(buf, 1000, asm_fp) != NULL) {
		if (infunc == 1 && buf[1] == '.') {
			infunc = 0;
		}
		
		if (strstr(buf, "@function") != NULL ) {
			func_count++;
			infunc = 1;
		}
	}
	function_table = calloc(func_count, sizeof(int *));
	funcname_table = calloc(func_count, sizeof(char *));
	rewind(asm_fp);

	char* funcname = NULL;
	// functionの名前、instructionの数等を確認
	// 各functionのgadget tableを作成
	while(fgets(buf, 1000, asm_fp) != NULL){
	
		if (infunc == 1 && buf[1] == '.') {
			make_function_gadget_offsets(funcnumber ,instruction_count, func_count, funcname, outasm_fp, obj_fp);
			funcnumber++;
			infunc = 0;
		}

		if (infunc == 1 && buf[0] == '\t') {
			instruction_count++;
		}

		if (strstr(buf, "@function") != NULL) {
			infunc = 1;
			instruction_count = 0;
			char* address = strstr(buf, ".type");
			address += 6;
			funcname_table[funcnumber] = malloc(100);
			funcname = (char *)funcname_table[funcnumber];
			for(int i = 0; ; i++) {
				if (address[i] == ',') {
					funcname[i] = '\0';
					break;
				}
				funcname[i] = address[i];
			}
		}
	}
	return;
}
