#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>

unsigned char SEED1[33] = "\x37\x28\x26\x68\xbc\x6c\x43\xb9\x66\x5b\x45\xcf\xb0\xe5\x4b\xc7\xde\xa4\x7c\xd\xa2\x80\x95\xeb\xe4\x55\x74\x6f\x82\x5a\xbe\x62\x00";


int func(char* buf, int check) {
    unsigned char SEED2[33] = "\x12\x18\x10\x48\x99\x49\x49\x93\x48\x59\x59\xf3\xa8\xe8\x49\xd4\xc6\xb5\x68\x0a\x8c\x85\x96\xe7\xf8\x69\x57\x43\xa3\x76\x8d\x7c\x00";
    unsigned char odd = 'c';
    unsigned char even = 'q';
    for (int i = 0;i <  32; i++) {
        if (i % 2 == 1) {
            if ((buf[i] ^ odd) != (SEED1[i] ^ SEED2[i])) {
                check = 1;
                break;
            }
        } else {
            if ((buf[i] ^ even) != (SEED1[i] ^ SEED2[i])) {
                check = 1;
                break;
            }
        }
        continue;
    }
    return check;
}


int main(){
    int check = 0;
    char buf[33];
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1) {
        exit(1);
    }
    puts("FLAG >");
    scanf("%32s", buf);
    check = func(buf, check);
    if (check == 0) {
        printf("correct!");
    } else {
        printf("wrong!");
    }
    return 0;
}
