//
//  main.c
//
//
//  Created by Antony Hwang on 04/11/2016.
//
//

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAX_BUFF_SIZE 1024

char path[MAX_BUFF_SIZE] = "";
char home[MAX_BUFF_SIZE] = "";

char **paths;

int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

bool loadConfigFile() {
    char *buffer;
    buffer = malloc(sizeof(char) * MAX_BUFF_SIZE);
    FILE *pfile;
    char *filename = "profile";
    int pathAssigned = 0;
    int homeAssigned = 0;
    pfile = fopen(filename, "r");
    if (pfile == NULL) {
        printf("File \"%s\" does not exist\n", filename);
        exit(1);
    }
    while(1) {
        fgets (buffer, MAX_BUFF_SIZE, pfile);
        if (prefix("PATH=", buffer)) {
            strncpy(path, buffer + 5, strlen(buffer) - 5);
        }
        else if (prefix("HOME=", buffer)) {
            strncpy(home, buffer + 5, strlen(buffer) - 5);
        }
        if (feof(pfile)) {
            if (strlen(path) == 0 || strlen(home) == 0) {
                return false;
            }
            else {
                return true;
            }
        }
        free(buffer);
    }
    fclose(pfile);
}

void getSearchPath() {
    char *token;
    int counter = 0;
    paths = malloc(sizeof(char) * MAX_BUFF_SIZE);
    token = strtok(path, ":");
    while (token != NULL)
    {
        paths[counter] = token;
        counter++;
        token = strtok(NULL, "\n:");
    }
    paths[counter] = " ";
}

void cd(char **arg) {
    char cwd[MAX_BUFF_SIZE] = "";
    if (strcmp(arg[1], " ") == 0) {
        strcat(cwd, home);
    }
    else {
        getcwd(cwd, sizeof(cwd));
        strcat(cwd, "/");
        strcat(cwd, arg[1]);
    }
    int status = chdir(cwd);
    if (status != 0) {
        printf("Failed to change directory\n");
        printf("No such file or directory\n");
    }
}

int launchCommand(char *buffer, char **command) {
    pid_t pid = fork();

    if (pid == -1)
    {
        printf("fork failed");
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
    }
    else
    {
        execv(buffer, command);
    }
    return 1;
}

int runProgramArg() {
    char *input = malloc( MAX_BUFF_SIZE * sizeof( char* ));
    char **command = malloc( MAX_BUFF_SIZE * sizeof( char* ));
    fgets(input, MAX_BUFF_SIZE, stdin);
    char *token;
    int counter = 0;
    token = strtok(input, " \n\t");
    while (token != NULL)
    {
        command[counter] = token;
        counter++;
        token = strtok(NULL, " \n\t");
    }
    if (strcmp(command[0], "cd") == 0) {
        command[counter] = " ";
        cd(command);
        free(command);
        free(input);
        return 1;
    }
    else {
        for (int i = 0; i < MAX_BUFF_SIZE; i++) {
            char buffer[MAX_BUFF_SIZE] = "";
            strcpy(buffer, paths[i]);
            strcat(buffer, "/");
            strcat(buffer, command[0]);
            if (access(buffer, X_OK) == 0) {
                int status = launchCommand(buffer, command);
                free(command);
                free(input);
                return status;
            }
            if (strcmp(paths[i], " ")) {
                break;
            }
        }
    }
    printf("command not found\n");
    free(command);
    free(input);
    return 1;
}

void runShell () {

    int commandInput = 0;
    getSearchPath();
    do {
        char cwd[MAX_BUFF_SIZE] = "";
        commandInput = 0;
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s>", cwd);
        }
        else {
            perror("Error: current directory not found");
        }
        commandInput = runProgramArg();
    } while (commandInput);
}

int main() {
    if (loadConfigFile() == false) {
        printf("profile does not exist or if either variable is not assigned");
        return 1;
    }
    else {
        chdir(home);
        runShell();
    }
  return 1;
}
