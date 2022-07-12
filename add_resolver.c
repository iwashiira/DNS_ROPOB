#include "dns_ropob.h"

void asm_write(char *str, FILE *outasm_fp) {
	char buf[1001];
	strncpy(buf, str, 1000);
	fwrite(buf, sizeof(char), strlen(buf), outasm_fp);
}

void add_resolver(FILE *outasm_fp) {
	asm_write("\t.text\n", outasm_fp);
	asm_write("\t.globl\tresolver\n",outasm_fp);
	asm_write("\t.type\tresolver, @function\n",outasm_fp);
	asm_write("resolver:\n",outasm_fp);
	asm_write("\tpushfq\n",outasm_fp);
	asm_write("\tpushfq\n",outasm_fp);
	asm_write("\tpush\trax\n",outasm_fp);
	asm_write("\tpush\trcx\n",outasm_fp);
	asm_write("\tpush\trdx\n",outasm_fp);
	asm_write("\tmov\trax, QWORD PTR funcnumber[rip]\n",outasm_fp);
	asm_write("\tlea\trcx, funcgadgetoffsets[rip]\n",outasm_fp);
	asm_write("\tmov\trdx, QWORD PTR [rcx+rax*8]\n",outasm_fp);
	asm_write("\tmov\trax, rdx\n",outasm_fp);
	asm_write("\tmov\trcx, QWORD PTR gadgetnumber[rip]\n",outasm_fp);
	asm_write("\tadd\trcx, 1\n",outasm_fp);
	asm_write("\tmov\trdx, QWORD PTR [rax+rcx*8]\n",outasm_fp);
	asm_write("\tmov\trax, QWORD PTR base[rip]\n",outasm_fp);
	asm_write("\tadd\trax, rdx\n",outasm_fp);
	asm_write("\tmov\tQWORD PTR [rsp+32], rax\n",outasm_fp);
	asm_write("\tpop\trdx\n",outasm_fp);
	asm_write("\tpop\trcx\n",outasm_fp);
	asm_write("\tpop\trax\n",outasm_fp);
	asm_write("\tpopfq\n",outasm_fp);
	asm_write("\tret\n",outasm_fp);
}
