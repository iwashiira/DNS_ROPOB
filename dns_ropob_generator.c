#include "dns_ropob.h"

void error(char* error_msg, int error_number) {
	fprintf(stderr, "%s\n", error_msg);
	exit(error_number);
}

int main(int argc, char* argv[]) {

	// check input file is .c or not
	if (argc < 2) {
		error("Input file must be required", 1);
	}
	const char *filename = argv[1];
	const char *ext = strrchr(filename, '.');
	char filename_noext[24];
	if (ext == 0 || strncmp(ext, ".c\0", 3) != 0) {
		error("Input file must be \".c\"", 2);
	}
	if (strlen(filename) > 25) {
		error("Input filename is too long. Not extend 25 chars", 3);
	}

	// compile input file
	size_t length = (size_t)(ext - filename);
	strncpy(filename_noext, filename, length);
	filename_noext[length] = '\0';
	
	char gcc_genasm[123];
	sprintf(gcc_genasm, "gcc -O0 -S -o %s.s %s -masm=intel -fno-asynchronous-unwind-tables -mno-red-zone", filename_noext, filename);
	char gcc_genexec[91];
	sprintf(gcc_genexec, "gcc -o %s %s.s", filename_noext, filename_noext);
	char obj_cmd[97];
	sprintf(obj_cmd, "objdump -d -M intel --target=elf64-x86-64 %s > %s1.obj", filename_noext, filename_noext);
	
	int compile_status = system(gcc_genasm);
	if (compile_status != 0) {
		error("gcc can't compile input file", 4);
	}
	compile_status = system(gcc_genexec);
	if (compile_status != 0) {
		error("gcc can't generate input exec for objdump", 5);
	}
	int disas_status = system(obj_cmd);
	if (disas_status != 0) {
		error("objdump can't generate disas result", 6);
	}
	
	// open input and output asm file and obj file
	char asm_file[26];
	char outasm1_file[30];
	char obj1_file[29];
	sprintf(asm_file, "%s.s", filename_noext);
	sprintf(outasm1_file, "%sout1.s", filename_noext);
	sprintf(obj1_file, "%s1.obj", filename_noext);
	asm_fp = fopen(asm_file, "r");
	outasm1_fp = fopen(outasm1_file, "w");
	obj1_fp = fopen(obj1_file, "r");
	if (asm_fp == NULL) {
		error("Can't open asm_file", 7);
	}
	if (outasm1_fp == NULL) {
		error("Can't open asmout1_file", 8);
	}
	if (obj1_fp == NULL) {
		error("Can't open obj1_file", 9);
	}
	char asm[100];
	for (int i = 0;i < 3; i++) {
		fgets(asm, sizeof(asm), asm_fp);
		fwrite(asm, sizeof(char), strlen(asm), outasm1_fp);
	}

	// make outasm1.s
	// this .s missed jmp instructions length
	make_global("base",outasm1_fp);
	make_global("funcnumber",outasm1_fp);
	make_global("gadgetnumber",outasm1_fp);
	init_function_info(asm_fp, outasm1_fp, obj1_fp);
	add_resolver(outasm1_fp);
	copy_asm_to_outasm(asm_fp, outasm1_fp);
	
	fclose(asm_fp);
	fclose(outasm1_fp);
	fclose(obj1_fp);
	
	sprintf(gcc_genexec, "gcc -O0 -o %s %s", filename_noext, outasm1_file);
	sprintf(obj_cmd, "objdump -d -M intel --target=elf64-x86-64 %s > %s2.obj", filename_noext, filename_noext);
	
	compile_status = system(gcc_genasm);
	if (compile_status != 0) {
		error("gcc can't compile input file", 4);
	}
	compile_status = system(gcc_genexec);
	if (compile_status != 0) {
		error("gcc can't generate input exec for objdump", 5);
	}
	disas_status = system(obj_cmd);
	if (disas_status != 0) {
		error("objdump can't generate disas result", 6);
	}
	char outasm2_file[30];
	char obj2_file[29];
	sprintf(outasm2_file, "%sout2.s", filename_noext);
	sprintf(obj2_file, "%s2.obj", filename_noext);
	outasm1_fp = fopen(outasm1_file, "r");
	outasm2_fp = fopen(outasm2_file, "w");
	obj2_fp = fopen(obj2_file, "r");
	if (outasm1_fp == NULL) {
		error("Can't open asmout1_file", 8);
	}
	if (outasm2_fp == NULL) {
		error("Can't open asmout2_file",8);
	}
	if (obj2_fp == NULL) {
		error("Can't open obj2_file", 9);
	}

	copy_outasm1_to_outasm2(outasm1_fp, outasm2_fp, obj2_fp);

	fclose(outasm1_fp);
	fclose(outasm2_fp);
	fclose(obj2_fp);
	
	sprintf(gcc_genexec, "gcc -O0 -o %s %s", filename_noext, outasm2_file);
	compile_status = system(gcc_genexec);
	if (compile_status != 0) {
		error("gcc can't compile out.s file", 10);
	}

	// 不要なファイルを削除
	char rm_cmd[58];
	sprintf(rm_cmd, "rm %s*.obj %s*.s", filename_noext, filename_noext);
	int rm_status = system(rm_cmd);
	if (rm_status != 0) {
		error("Can't delete junk files", 11);
	}

	return 0;
}
