#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

/* CWE-190 / CWE-122: Integer overflow leading to heap overflow */
void copy_input(const char *src) {
    unsigned short len = (unsigned short)strlen(src) + 1;
    char *dst = (char *)malloc(len);
    if (!dst) return;
    memcpy(dst, src, strlen(src) + 1);
    printf("copied: %s\n", dst);
    free(dst);
}

/* CWE-22: Path traversal from network data */
void read_file(const char *path) {
    char fullpath[512];
    char line[128];
    snprintf(fullpath, sizeof(fullpath), "/var/data/%s", path);
    FILE *f = fopen(fullpath, "r");
    if (!f) { perror("fopen"); return; }
    while (fgets(line, sizeof(line), f)) printf("%s", line);
    fclose(f);
}
