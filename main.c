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

char path[80];
char home[80];
char *currentWorkingDir;

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
            printf("path: %s", path);
        }
        else if (prefix("HOME=", buffer)) {
            strncpy(home, buffer + 5, strlen(buffer) - 5);
            printf("home: %s", home);;
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

char *getInput() {
    
}

void runCommand () {
    bool commandInput;
    do {
        printf("%s>", currentWorkingDir);
        readInput =
        getProgram =
        getArg =
        findProgram =
        forkProgram =
        executeProgram
    } while (commandInput);
}

int main() {
    if (loadConfigFile() == false) {
        printf("profile does not exist or if either variable is not assigned");
        return 1;
    }
    else {
        currentWorkingDir = home;
        runCommand();
    }
  return 1;
}
