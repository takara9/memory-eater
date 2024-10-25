#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include<fcntl.h>

char *memory_eater(int size) {
    unsigned int sz = 1024*1024 * size; // MB
    char *p = malloc(sz);
    char *pp = p;
    unsigned int i;
    memset(p, 0x41, sz);
    return p;
}

int sleep2(int sleep_sec) {
    int fd, ret;
    fd_set rfds;
    struct timeval tv;
    char* inputval;

    fd = open("/tmp/xxx", O_RDONLY|O_CREAT);

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    if (sleep_sec > 0) {
        tv.tv_sec = sleep_sec;
        ret = select(fd + 1, &rfds, NULL, NULL, &tv);
    } else {
        ret = select(fd + 1, &rfds, NULL, NULL, NULL);
    }
    if (ret < 0) {
        perror("select()");
    }
}

void sigterm_handler(int sig) {
    exit(0);
}

/*
    100Miづつ、指定のインターバルで、追加する
    mem-eater 1 3 20  100Mi から 300Mi まで20秒のインターバル
    mem-eater 10 20 10  1Gi から 2Gi　まで10秒のインターバル
*/
int main(int argc, char *argv[]) {
    char *p[1024];
    unsigned int init_size_mb, last_size_mb, sleep_time, i, j, current_size;

    signal(SIGTERM, sigterm_handler);

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
        sleep2(sleep_time);
    }
    sleep2(0);
}