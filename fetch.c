#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#define BUFFER_SIZE 32

//takes a matrix as parameter and put the strings at the right places
void get_kernel_info(char kernel_info[3][BUFFER_SIZE]) {
    char buffer[128];

    FILE *fp = popen("uname -srm", "r");
    if (fp == NULL) {
        perror("Could not open pipe for output.\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Returns the first token
        char *token = strtok(buffer, " ");
        for (unsigned int i = 0; token != NULL; i++) {
            strncpy(kernel_info[i], token, BUFFER_SIZE);
            token = strtok(NULL, " ");
        }
        //remove the \n at the end of the last token
        kernel_info[2][strlen(kernel_info[2]) - 1] = '\0';
    } else {
        perror("Failed to read to the buffer.\n");
        exit(EXIT_FAILURE);
    }

    pclose(fp);
}

void get_os(char os[BUFFER_SIZE]) {
    FILE *file = fopen("/etc/os-release", "r");
    if (file == NULL) {
        perror("Couldn't open file /etc/os-release");
    }
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), file)) {
        //get a pointer to the first ocurrance of PRETTY_NAME
        if(strstr(buffer, "PRETTY_NAME=")) {
            char *token = strtok(buffer, "\"");
            token = strtok(NULL, "\"");
            strncpy(os, token, BUFFER_SIZE);
            break;
        }
    }
}

int main(void) {
    //print os name
    char os[BUFFER_SIZE];
    get_os(os);
    printf("%s\n", os);
    
    //print kernel information
    char kernel_info[3][BUFFER_SIZE];
    get_kernel_info(kernel_info);

    printf("Kernel Name: %s\n", kernel_info[0]);
    printf("Kernel Release: %s\n", kernel_info[1]);
    printf("Machine: %s\n", kernel_info[2]);

    //print terminal emulator name
    char *term = getenv("TERM");
    if (term != NULL) 
        printf("Terminal emulator: %s\n", term);
    else
        printf("Unknown");
}
