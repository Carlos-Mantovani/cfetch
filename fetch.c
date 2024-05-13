#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#define BUFFER_SIZE 32

#define RED "\033[1;31m"
#define RESET "\033[1;0m"

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

void get_command_output(char buffer[32], char *command) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Chould not open pipe for output.\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(buffer, 32, fp) == NULL) {
        perror("Failed to write to the buffer.\n");
        exit(EXIT_FAILURE);
    }
    buffer[strlen(buffer) - 1] = '\0';
}

void get_shell(char shell_v[32]) {
    //get shell env
    char *shell = getenv("SHELL");
    shell += strlen("\\bin\\");

    //execute shell env plus "--version" to get the version
    char command[50];
    strncpy(command, shell, 50);
    strcat(command, " --version"); 
    FILE *fp = popen(command, "r");
    char buffer[50];
    if (fgets(buffer, 50, fp) != NULL)
        buffer[strlen(buffer) - 1] = '\0';
    else {
        perror("Failed to write to buffer.\n");
        exit(EXIT_FAILURE);
    }
    //Get rid of what's after the shell version
    char *end = strchr(buffer, '(');
    int length = end - buffer;
    strncpy(shell_v, buffer, length);
    shell_v[length - 1] = '\0';
}

int main(void) {
    //print username and hostname
    char user[32];
    get_command_output(user, "whoami");

    char host[32];
    get_command_output(host, "hostname");
    
    printf("%s%s%s@%s%s%s\n", RED, user, RESET, RED, host, RESET);
 
    //print os name
    char os[BUFFER_SIZE];
    get_os(os);
    printf("%sOS%s: %s\n", RED, RESET, os);
    
    //print kernel information
    char kernel_info[3][BUFFER_SIZE];
    get_kernel_info(kernel_info);

    printf("%sKernel%s: %s\n", RED, RESET, kernel_info[0]);
    printf("%sRelease%s: %s\n", RED, RESET, kernel_info[1]);
    printf("%sMachine%s: %s\n", RED, RESET, kernel_info[2]);

    //print terminal emulator name
    char *term = getenv("TERM");
    if (term != NULL) 
        printf("%sTerminal%s: %s\n", RED, RESET, term);
    else
        printf("Unknown");

    //print the currently used shell and version
    char shell_v[32];
    get_shell(shell_v);
    printf("%sShell%s: %s\n", RED, RESET, shell_v);
}
