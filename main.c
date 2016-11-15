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
            strtok(path, "\n");
        }
        else if (prefix("HOME=", buffer)) {
            strncpy(home, buffer + 5, strlen(buffer) - 5);
            strtok(home, "\n");
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
    char *pathCopy = malloc( MAX_BUFF_SIZE * sizeof( char* ));
    strcpy(pathCopy, path);
    paths = malloc(sizeof(char) * MAX_BUFF_SIZE);
    token = strtok(pathCopy, "\n:");
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
        printf("%s\n", cwd);
    }
    int status = chdir(cwd);
    if (status != 0) {
        printf("No such file or directory\n");
    }
}

void chHome(char *arg) {
    char *newHome = malloc(sizeof(arg) - 5);
    strncpy(newHome, arg + 6, strlen(arg) - 6);
    strtok(newHome, "\n");
    strcpy(home, newHome);
}

void chPath(char *arg) {
    char *newPath = malloc(sizeof(arg) - 5);
    strncpy(newPath, arg + 6, strlen(arg) - 6);
    strtok(newPath, "\n");
    strcpy(path, newPath);
}

int launchCommand(char *buffer, char **command) {
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        // Child process
        execv(buffer, command);
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
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
    if (strcmp(input, "\n") == 0) {
        return 1;
    }
    while (token != NULL) {
        command[counter] = token;
        counter++;
        token = strtok(NULL, " \n\t");
    }
    if (strcmp(command[0], "cd") == 0) {
        command[counter] = " ";
        cd(command);
        return 1;
    }
    else if (strncmp(command[0], "$HOME=", 6) == 0) {
        chHome(command[0]);
        return 1;
    }
    else if (strncmp(command[0], "$PATH=", 6) == 0) {
        chPath(command[0]);
        return 1;
    }
    else if (strcmp(command[0], "$HOME") == 0) {
        printf("%s: is a directory\n", home);
        return 1;
    }
    else if (strcmp(command[0], "$PATH") == 0) {
        printf("%s\n", path);
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
    char cwd[MAX_BUFF_SIZE];
    int commandInput = 0;
    getSearchPath();
    do {
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
