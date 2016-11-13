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

char path[MAX_BUFF_SIZE];
char home[MAX_BUFF_SIZE];

char **paths;

int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

bool loadConfigFile() {
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
        char buffer[MAX_BUFF_SIZE];
        
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
}

int runProgramArg() {
    int found = 0;
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
    for (int i = 0; i < MAX_BUFF_SIZE; i++) {
        char *buffer;
        buffer = malloc(sizeof(char) * MAX_BUFF_SIZE);
        strcpy(buffer, paths[i]);
        strcat(buffer, "/");
        strcat(buffer, command[0]);
        if (strlen(paths[i]) == 0) {
            break;
        }
        
        if (access(buffer, X_OK) == 0) {
            printf("found\n");
            if (fork() != 0) {
                wait(NULL);
            }
            else {
                execv(buffer, command);
            }
            found = 1;
            free(buffer);
            free(command);
            free(input);
            break;
        }
    }
    if (found == 0) {
        printf("command not found\n");
        free(command);
        free(input);
    }
    if (fork() != 0) {
        wait(NULL);
    }
    return 1;
}

void runShell () {
    bool commandInput = 0;
    getSearchPath();
    do {
        commandInput = 0;
        char cwd[MAX_BUFF_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            fprintf(stdout, "%s>", cwd);
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
