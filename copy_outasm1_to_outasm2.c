#include "dns_ropob.h"

void copy_outasm1_to_outasm2(FILE* outasm1_fp, FILE* outasm2_fp, FILE* obj2_fp) {
	char asm[1001];
	rewind(outasm1_fp);
	rewind(outasm2_fp);
	rewind(obj2_fp);
	int func_number = 0;
	while(fgets(asm, 1000, outasm1_fp) != NULL) {
		if (strstr(asm, "gadgettable:") != NULL) {
			fwrite(asm, sizeof(char), strlen(asm), outasm2_fp);
			make_function_gadget_table_fix_jmp(outasm1_fp, outasm2_fp, obj2_fp, func_number);
			func_number++;
		} else {
			fwrite(asm, sizeof(char), strlen(asm), outasm2_fp);
		}
	}
	return;
}
