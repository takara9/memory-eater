#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>

char *memory_eater(int size) {
    unsigned int sz = 1024*1024 * size; // MB
    //printf("alloc %d", size);
    char *p = malloc(sz);
    char *pp = p;
    unsigned int i;
    memset(p, 0x41, sz);
    return p;
}

/*
    100Miづつ、指定のインターバルで、追加する
    mem-eater 1 3 20  100Mi から 300Mi まで20秒のインターバル
    mem-eater 10 20 10  1Gi から 2Gi　まで10秒のインターバル
*/
int main(int argc, char *argv[]) {
    char *p[1024];
    unsigned int init_size_mb, last_size_mb, sleep_time, i, j, current_size;
    init_size_mb = atoi(argv[1]);
    last_size_mb = atoi(argv[2]);
    sleep_time = atoi(argv[3]);
    current_size = init_size_mb *100;
    j = last_size_mb - init_size_mb;
    for (i=0;i <= j;i++) {
        if (i == 0) {
            p[i] = memory_eater(init_size_mb * 100);
        } else {
            p[i] = memory_eater(100);
            current_size = current_size + 100;
        }
        printf("i= %d\t %d\n", i, current_size);
        sleep(sleep_time);
    }
    sleep(600);
}