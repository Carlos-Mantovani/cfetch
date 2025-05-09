#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#define BUFFER_SIZE 32
#define LARGER_BUFFER_SIZE 64
#define CMD_BUFFER_SIZE 128
#define OS_NAME_LENGTH 64

#define RED "\033[1;31m"
#define RESET "\033[1;0m"

//takes a matrix as parameter and put the strings at the right places
void get_kernel_info(char kernel_info[3][BUFFER_SIZE]) {
    char buffer[CMD_BUFFER_SIZE];
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
            kernel_info[i][BUFFER_SIZE - 1] = '\0';
            token = strtok(NULL, " ");
        }
        //remove the \n at the end of the last token
        kernel_info[2][strlen(kernel_info[2]) - 1] = '\0';
    } else {
        perror("Failed to read to the buffer.\n");
        pclose(fp);
        exit(EXIT_FAILURE);
    }

    pclose(fp);
}

void get_os(char os[OS_NAME_LENGTH]) {
    FILE *fp = fopen("/etc/os-release", "r");
    if (fp == NULL) {
        perror("Couldn't open file /etc/os-release");
        strncpy(os, "Unknown", OS_NAME_LENGTH);
        return;
    }
    char buffer[OS_NAME_LENGTH];
    while (fgets(buffer, sizeof(buffer), fp)) {
        //get a pointer to the first ocurrance of PRETTY_NAME
        if(strstr(buffer, "PRETTY_NAME=")) {
            char *token = strtok(buffer, "\"");
            token = strtok(NULL, "\"");
            strncpy(os, token, BUFFER_SIZE);
            os[BUFFER_SIZE - 1] = '\0';
            break;
         }
    }
    fclose(fp);
}

void get_command_output(char *buffer, size_t buffer_size, char *command) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Chould not open pipe for output.\n");
        exit(EXIT_FAILURE);
    }

    if (fgets(buffer, buffer_size, fp) == NULL) {
        perror("Failed to write to the buffer.\n");
        pclose(fp);
        exit(EXIT_FAILURE);
    }
    buffer[strlen(buffer) - 1] = '\0';

    pclose(fp);
}

void get_uptime(char buffer[BUFFER_SIZE]) {
    get_command_output(buffer, LARGER_BUFFER_SIZE, "uptime -p");
    unsigned int i = 0;
    do {
        buffer[i] = buffer[i + 3];
        i++;
    } while (buffer[i - 1] != '\0');
}

void get_shell(char shell_v[BUFFER_SIZE]) {
    //get shell env
    char *shell_path = getenv("SHELL");
    if (!shell_path) {
        strncpy(shell_v, "Unknown", BUFFER_SIZE);
        return;
    }
    //execute shell env plus "--version" to get the version
    char command[CMD_BUFFER_SIZE];
    snprintf(command, CMD_BUFFER_SIZE, "%s --version", shell_path);

    char buffer[CMD_BUFFER_SIZE];
    get_command_output(buffer, BUFFER_SIZE, command);

    char *newline = strchr(buffer, '\n');
    if (newline) *newline = '\0';

    char *paren = strchr(buffer, '(');
    if (paren) *paren = '\0';

    strncpy(shell_v, buffer, BUFFER_SIZE);
    shell_v[BUFFER_SIZE - 1] = '\0';
}

void get_packages(char buffer[BUFFER_SIZE * 2], char *os) {
    //char *os_name = strtok(os, " ");
    //char *os_second_name = strtok(NULL, " ");
    if (strstr(os, "Fedora")) {
        char dnf[50];
        char rpm[50];
        char flatpak[50];
        get_command_output(dnf, BUFFER_SIZE, "dnf repoquery --installed | wc -l");
        get_command_output(rpm, BUFFER_SIZE, "rpm -qa | wc -l");
        get_command_output(flatpak, BUFFER_SIZE, "flatpak list | wc -l");
        snprintf(buffer, BUFFER_SIZE * 2, "%s (dnf), %s (rpm), %s (flatpak)", dnf, rpm, flatpak);
    }
    if (strstr(os, "Ubuntu")) {
        char apt[50];
        char snap[50];
        get_command_output(apt, BUFFER_SIZE , "dpkg --get-selections | wc -l");
        get_command_output(snap, BUFFER_SIZE , "snap list | wc -l");
        snprintf(buffer, BUFFER_SIZE * 2, "%s (apt), %s (snap)", apt, snap);
    }
    if (strstr(os, "Linux Mint") || strstr(os, "Pop!_OS")) {
        char apt[50];
        get_command_output(apt, BUFFER_SIZE, "dpkg --get-selections | wc -l");
        snprintf(buffer, BUFFER_SIZE * 2, "%s (apt)", apt);
    }
    if(strstr(os, "Arch")) {
        char pacman[50];
        char AUR[50];
        get_command_output(pacman, BUFFER_SIZE, "pacman -Q | wc -l");
        get_command_output(AUR, BUFFER_SIZE, "yay -Qm | wc -l");
        snprintf(buffer, BUFFER_SIZE * 2, "%s (pacman), %s (AUR)", pacman, AUR);
    }
}

void get_terminal(char terminal[BUFFER_SIZE]) {
    char command[CMD_BUFFER_SIZE] = "ps -o comm= -p $(ps -o ppid= -p $(ps -o ppid= -p $(ps -o ppid= -p $$)))";
    get_command_output(terminal, BUFFER_SIZE, command);

    if (strlen(terminal) == 0) {
        strncpy(terminal, "Unknown", BUFFER_SIZE);
        terminal[BUFFER_SIZE - 1] = '\0';
    }
}

void get_cpu(char cpu[LARGER_BUFFER_SIZE * 2]) {
    /*
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) {
        perror("Couldn't open file /proc/cpuinfo");
        strncpy(cpu, "Unknown", LARGER_BUFFER_SIZE);
        return;
    }
    char buffer[LARGER_BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strncmp(buffer, "model name", 10) == 0) {
            char *token = strstr(buffer, ": ");
            token += 2;
            token[strcspn(token, "\n")] = '\0';
            strncpy(cpu, token, LARGER_BUFFER_SIZE);
            cpu[LARGER_BUFFER_SIZE - 1] = '\0';
            break;
        }
    }
    fclose(fp);
    */
    get_command_output(cpu, LARGER_BUFFER_SIZE * 2, "grep 'model name' /proc/cpuinfo | head -n1 | cut -d':' -f2-");
    //char *trim = cpu;
    //trim++;
    memmove(cpu, cpu+1, strlen(cpu) + 1);
}

void get_gpu(char gpu[LARGER_BUFFER_SIZE * 2]) {
    get_command_output(gpu, LARGER_BUFFER_SIZE * 2, "lspci | grep VGA | cut -d':' -f3-");
    memmove(gpu, gpu+1, strlen(gpu) + 1);
}

int main(void) {
    //get username and hostname
    char user[BUFFER_SIZE];
    get_command_output(user, BUFFER_SIZE, "whoami");
    char host[BUFFER_SIZE];
    get_command_output(host, BUFFER_SIZE, "hostname");
    //get os name
    char os[OS_NAME_LENGTH];
    get_os(os);
    //get kernel information
    char kernel_info[3][BUFFER_SIZE];
    get_kernel_info(kernel_info);
    //get system uptime
    char uptime[BUFFER_SIZE];
    get_uptime(uptime);
    //find the number of installed packages
    char packages[BUFFER_SIZE * 2];
    get_packages(packages, os);
    //get terminal emulator name
    char terminal[BUFFER_SIZE];
    get_terminal(terminal);
    //get the currently used shell and version
    char shell_v[32];
    get_shell(shell_v);
    //get cpu model
    char cpu[LARGER_BUFFER_SIZE * 2];
    get_cpu(cpu);
    //get gpu model
    char gpu[LARGER_BUFFER_SIZE * 2];
    get_gpu(gpu);

    printf("%s%s%s@%s%s%s\n", RED, user, RESET, RED, host, RESET);
    printf("%sOS%s: %s\n", RED, RESET, os);
    printf("%sKernel%s: %s %s %s\n", RED, RESET, kernel_info[0], kernel_info[1], kernel_info[2]);
    printf("%sUptime%s: %s\n", RED, RESET, uptime);
    printf("%sPackages%s: %s\n", RED, RESET, packages);
    printf("%sTerminal%s: %s\n", RED, RESET, terminal);
    printf("%sShell%s: %s\n", RED, RESET, shell_v);    
    printf("%sCPU%s: %s\n", RED, RESET, cpu);
    printf("%sGPU%s: %s\n", RED, RESET, gpu);
}
