#include "dns_ropob.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void shuffle(int array[], int size) {
	unsigned int seed;
	int fd;
	fd = open("/dev/urandom", O_RDONLY);
	if (fd) {
		read(fd, &seed, sizeof(seed));
		close(fd);
		srand(seed);
		for(int i = 0; i < size; i++) {
			int j = rand()%size;
			int t = array[i];
			array[i] = array[j];
			array[j] = t;
		}
	}
}

void make_func_offs_table(int offsets, FILE *outasm_fp) {
	char buf[100];
	sprintf(buf, "\t.quad\t%d\n", offsets);
	fwrite(buf, sizeof(char), strlen(buf), outasm_fp);
	return;
}

void make_function_gadget_table(int funcnumber, int instruction_count, char* funcname, FILE* outasm_fp, FILE* obj_fp) {
	function_table[funcnumber] = calloc(4,sizeof(int *));
	int* gadget = calloc(instruction_count, sizeof(int));
	int* gadget_inst_length = calloc(instruction_count, sizeof(int));
	int* gadget_offsets = calloc(instruction_count, sizeof(int));
	int* gadget_count = calloc(1, sizeof(int));
	function_table[funcnumber][0] = gadget;
	function_table[funcnumber][1] = gadget_inst_length;
	function_table[funcnumber][2] = gadget_offsets;
	function_table[funcnumber][3] = gadget_count;

	*gadget_count = instruction_count;
	
	int infunc = 0;
	int space = 0;
	char buf[1001];
	for (int i = 0;i < instruction_count; i++) {
		gadget[i] = i;
	}
	
	char funcname_obj[1000];
	char old_offs[10] = "0x";
	char now_offs[10] = "0x";
	int gadget_number = 0;
	sprintf(funcname_obj, "<%s>:", funcname);
	// gadgetの機械語の長さを確認
	rewind(obj_fp);
	while (fgets(buf, 1000, obj_fp) != NULL) {
		if (infunc == 1) {
			for (int i = 0; i < 1001 ;i++) {
				if (buf[i] != ' ') {
					space = i;
					break;
				}
			}
			if (strrchr(buf, '\t')  - buf - 1 == strchr(buf, ':') - buf) {
				continue;
			}
			if (gadget_number != 0) {
				strncpy(old_offs, now_offs, 10);
			}
			for(int i = space, j = (strchr(buf, ':') - buf); i <= j; i++) {
				if (i == j) {
					now_offs[i+1] = '\0';
				}
				now_offs[i - space + 2] = buf[i];
			}
			if (gadget_number != 0) {
				gadget_inst_length[gadget_number - 1] = strtol(now_offs, NULL, 16) - strtol(old_offs, NULL, 16);
			}
			if (gadget_number == (instruction_count - 1)) {
				gadget_inst_length[gadget_number] = 1;
				infunc = 0;
			}
			gadget_number++;
		}

		if (strstr(buf, funcname_obj) != NULL) {
			infunc = 1;
		}
	}

	//gadget randomize
	size_t size = instruction_count - 1;
	int tmp[size];
	for (int i = 0; i < size; i++) {
		tmp[i] = gadget[i+1];
	}
	shuffle(tmp, size);
	for (int i = 0; i < size; i++) {
		gadget[i+1] = tmp[i];
	}
	
	//gadget_offsetsを計算
	int now;
	for (int i = 0; i < instruction_count; i++) {
		if(i == 0){
			if (strncmp(funcname, "main\0", 5) == 0) {
				gadget_offsets[i] = 17 + 18 * func_count;
			} else {
				gadget_offsets[i] = 0;
			}
			now = gadget_offsets[i];
		} else {
			now = now + gadget_inst_length[gadget[i - 1]] + 54;
			gadget_offsets[gadget[i]] = now;
		}
	}
	for (int i = 0; i < instruction_count; i++) {
		make_func_offs_table(gadget_offsets[i], outasm_fp);
	}

	return;
}

void make_function_gadget_table_fix_jmp(FILE * outasm1_fp, FILE *outasm2_fp, FILE *obj2_fp, int funcnumber) {
	char buf[1001];
	rewind(obj2_fp);
	int infunc = 0;
	int space = 0;
	int check = 0;
	int next = 0;
	char* funcname = funcname_table[funcnumber];
	char funcname_obj[1000];
	sprintf(funcname_obj, "<%s>:", funcname);
	char old_offs[10] = "0x";
	char now_offs[10] = "0x";
	char gadget_num[10] = "0x";
	int gadget_number = 0;
	int* gadget = function_table[funcnumber][0];
	int *gadget_inst_length = function_table[funcnumber][1];
	int* gadget_offsets = function_table[funcnumber][2];
	int* gadget_count = function_table[funcnumber][3];
	int instruction_count = *gadget_count;

	while(fgets(buf, 1000, obj2_fp) != NULL) {
		if (infunc == 1) {
			for (int i = 0; i < 1001 ;i++) {
				if (buf[i] == ' ') {
					continue;
				}
				space = i;
				break;
			}
			if (buf[0] != ' ' && buf[1] != ' ') {
				infunc = 0;
			}
			// jmpガジェットの機械語の長さを変更
			if (check == 1) {
				if (next == 2) {
					if (strstr(buf, "<gadgetnumber>") != NULL) {
						for (int i = 0; i < 10; i++) {
							gadget_num[i+2] = strchr(buf, ',')[i+3];
							if (gadget_num[i+2] == ' ') {
								gadget_num[i+2] = '\0';
								break;
							}
						}
						gadget_number = strtol(gadget_num, NULL, 16);
						gadget_inst_length[gadget_number] = strtol(now_offs, NULL, 16) - strtol(old_offs, NULL, 16);
						next = 0;
					}
				}
				if (next == 1) {
					for (int i = space, j = strchr(buf, ':') - buf; i < j ;i++) {
						now_offs[i - space + 2] = buf[i];
					}
					next = 2;
				}
				if (strstr(buf, "\tj") != NULL) {
					for (int i = space, j = strchr(buf, ':') - buf; i < j ;i++) {
						old_offs[i - space + 2] = buf[i];
					}
					next = 1;
				}
				if (next == 0) {
					check = 0;
				}
			}
			if (strstr(buf, "\tret") != NULL) {
				check = 1;
			}
		}
		if (strstr(buf, funcname_obj) != NULL) {
			infunc = 1;
		}
	}
	int now;
	for (int i = 0; i < instruction_count; i++) {
		if(i == 0){
			if (strncmp(funcname, "main\0", 5) == 0) {
				gadget_offsets[i] = 17 + 18 * func_count;
			} else {
				gadget_offsets[i] = 0;
			}
			now = gadget_offsets[i];
		} else {
			now = now + gadget_inst_length[gadget[i - 1]] + 54;
			gadget_offsets[gadget[i]] = now;
		}
	}	
	for (int i = 0; i < instruction_count; i++) {
		make_func_offs_table(gadget_offsets[i], outasm2_fp);
		fgets(buf, 1000, outasm1_fp);
	}
	return;
}
