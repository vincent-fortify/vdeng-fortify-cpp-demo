/* vuln-c-demo v2: deliberately vulnerable C code for Fortify SAST testing.
 * Tainted input comes from the network (recv), environment (getenv) and stdin (fgets),
 * which Fortify treats as untrusted sources. DO NOT deploy. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "util.h"

/* CWE-121: Stack-based buffer overflow (strcpy from network data) */
void greet(const char *name) {
    char buf[16];
    strcpy(buf, name);
    printf("Hello, %s\n", buf);
}

/* CWE-134: Format string from network data */
void log_message(const char *msg) {
    printf(msg);
    printf("\n");
}

/* CWE-78: OS command injection from environment variable */
void ping_host(void) {
    char cmd[256];
    const char *host = getenv("TARGET_HOST");
    if (!host) return;
    snprintf(cmd, sizeof(cmd), "ping -c 1 %s", host);
    system(cmd);
}

/* CWE-416: Use after free */
void use_after_free(void) {
    char *p = (char *)malloc(32);
    if (!p) return;
    strcpy(p, "temporary");
    free(p);
    printf("%s\n", p);
}

/* CWE-259 / CWE-798: Hard-coded password */
int check_admin(const char *input) {
    char *password = "SuperSecret123!";
    return strcmp(input, password) == 0;
}

/* Reads one request from a TCP client and dispatches it */
void handle_client(int client) {
    char request[1024];
    ssize_t n = recv(client, request, sizeof(request) - 1, 0);
    if (n <= 0) return;
    request[n] = '\0';

    if (strncmp(request, "greet ", 6) == 0) greet(request + 6);
    else if (strncmp(request, "log ", 4) == 0) log_message(request + 4);
    else if (strncmp(request, "copy ", 5) == 0) copy_input(request + 5);
    else if (strncmp(request, "file ", 5) == 0) read_file(request + 5);
    else if (strncmp(request, "admin ", 6) == 0) printf("%s\n", check_admin(request + 6) ? "ok" : "denied");
}

int main(void) {
    char line[64];
    int server, client;
    struct sockaddr_in addr;

    /* CWE-242: gets-style unbounded read via scanf %s from stdin */
    char name[16];
    printf("Operator name: ");
    if (scanf("%s", name) == 1) printf("Operator: %s\n", name);

    if (fgets(line, sizeof(line), stdin) && strncmp(line, "uaf", 3) == 0) use_after_free();
    ping_host();

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) return 1;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);
    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) return 1;
    listen(server, 5);
    client = accept(server, NULL, NULL);
    if (client >= 0) { handle_client(client); close(client); }
    close(server);
    return 0;
}
