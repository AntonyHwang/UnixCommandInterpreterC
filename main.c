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
#include <dirent.h>

#define MAX_BUFF_SIZE 1024

char path[80];
char home[80];

struct command {
    char *program;
    char *arg;
    char *arg2;
};

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
        char buffer[80];
        
        fgets (buffer, 80, pfile);
        //printf("%s\n", buffer);
        if (prefix("PATH=", buffer)) {
            strncpy(path, buffer + 5, strlen(buffer) - 5);
            //printf("path: %s", path);
        }
        else if (prefix("HOME=", buffer)) {
            strncpy(home, buffer + 5, strlen(buffer) - 5);
            //printf("home: %s", home);;
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

void cd_(struct command c) {
    char cwd[MAX_BUFF_SIZE];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcat(cwd, c.arg);
    
    int status = chdir(cwd);
    if (status != 0) {
        printf("Failed to change directory\n");
    }
    /*{
        printf("Failed to change directory\n");
    }*/
}

void ls_(struct command c) {
    
}

void cat_(struct command c) {
    FILE *file;
    char *filename = c.arg;
    file = fopen(filename, "r");
    if (file == NULL) {
        printf("File \"%s\" does not exist\n", filename);
        exit(1);
    }
    while(1) {
        char buffer[MAX_BUFF_SIZE];
        fgets (buffer, MAX_BUFF_SIZE, file);
        printf("%s/n", buffer);
    }
    fclose(file);
}

void runCommand(struct command c) {
    char *cd = "cd";
    char *ls = "ls";
    char *cat = "cat";
    if (strcmp(c.program, cd) == 0) {
        cd_(c);
    }
    else if (strcmp(c.program, ls) == 0) {
        ls_(c);
    }
    else if (strcmp(c.program, cat) == 0) {
        cat_(c);
    }
    else {
        printf("invalid command\n");
        exit(1);
    }
}

void getProgramArg() {
    char input[MAX_BUFF_SIZE];
    fgets(input, MAX_BUFF_SIZE, stdin);
    char *token;
    int wordCount = 1;
    struct command c;
    c.program = malloc(sizeof(char) * MAX_BUFF_SIZE);
    c.arg = malloc(sizeof(char) * MAX_BUFF_SIZE);
    c.arg2 = malloc(sizeof(char) * MAX_BUFF_SIZE);
    token = strtok(input, " \n\t");
    c.program = token;
    while (token != NULL)
    {
        wordCount++;
        token = strtok(NULL, " \n\t");
        //printf ("%s\n",token);
        if (wordCount == 2) {
            c.arg = token;
        }
        else if (wordCount == 3) {
            c.arg2 = token;
        }
    }
    runCommand(c);
}

void runShell () {
    bool commandInput = 0;
    do {
        commandInput = 0;
        struct command c;
        c.program = malloc(sizeof(char) * MAX_BUFF_SIZE);
        c.arg = malloc(sizeof(char) * MAX_BUFF_SIZE);
        c.arg2 = malloc(sizeof(char) * MAX_BUFF_SIZE);
        
        char cwd[MAX_BUFF_SIZE];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            fprintf(stdout, "%s>", cwd);
        }
        else {
            perror("Error: current directory not found");
        }
        getProgramArg();
        commandInput = 1;
        //args = getArgs(input);
        //findProgram =
        //forkProgram =
        //executeProgram
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
