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
//loads the profile file in the executable directory
bool loadConfigFile() {
    char *buffer;
    buffer = malloc(sizeof(char) * MAX_BUFF_SIZE);
    FILE *pfile;
    char *filename = "profile";
    int pathAssigned = 0;
    int homeAssigned = 0;
    //opens the file
    pfile = fopen(filename, "r");
    //check existence of file
    if (pfile == NULL) {
        printf("File \"%s\" does not exist\n", filename);
        exit(1);
    }
    //read line by line
    while(1) {
        fgets (buffer, MAX_BUFF_SIZE, pfile);
        //extract path and home
        //by checking the start of each line
        if (strncmp(buffer, "PATH=", 5) == 0) {
            strncpy(path, buffer + 5, strlen(buffer) - 5);
            strtok(path, "\n");
        }
        else if (strncmp(buffer, "HOME=", 5) == 0) {
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
    }
    free(buffer);
    fclose(pfile);
}
//extract search paths and separate them into tokens (array of strings)
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
//reading the arguments after cd
void cd(char **arg) {
    char cwd[MAX_BUFF_SIZE] = "";
    if (strcmp(arg[1], " ") == 0) {
        strcat(cwd, home);
    }
    else if (strcmp(arg[1], "/tmp") == 0) {
        strcpy(cwd, "//tmp");
    }
    else {
        getcwd(cwd, sizeof(cwd));
        strcat(cwd, "/");
        strcat(cwd, arg[1]);
    }
    //change the directory
    //check if its successful
    int status = chdir(cwd);
    if (status != 0) {
        printf("No such file or directory\n");
    }
}
//reading the arguments after $HOME=
void chHome(char *arg) {
    char *newHome = malloc(sizeof(arg) - 5);
    strncpy(newHome, arg + 6, strlen(arg) - 6);
    strtok(newHome, "\n");
    if (access(newHome, X_OK) == 0){
        //replace the old home directory with the new home directory
        strcpy(home, newHome);
        printf("New HOME assigned\n");
    }
    else {
        printf("HOME not assigned: home path does not exist\n");
    }
}
//reading the arguments after $PATH=
void chPath(char *arg) {
    char *newPath = malloc(sizeof(arg) - 5);
    strncpy(newPath, arg + 6, strlen(arg) - 6);
    strtok(newPath, "\n");
    //replace the old path with the new path
    strcpy(path, newPath);
    printf("New PATH assigned\n");
}

int launchCommand(char *buffer, char **command) {
    //get the current process id
    pid_t pid;
    int status;
    //duplicate the process
    pid = fork();
    if (pid == 0) {
        // child process
        execv(buffer, command);
    } else {
        // parent process
        do {
            waitpid(pid, &status, WUNTRACED);
            //wait until the child process finishes
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    
    return 1;
}
//reading the command input by the user
int runProgramArg() {
    char *input = malloc( MAX_BUFF_SIZE * sizeof( char* ));
    char **command = malloc( MAX_BUFF_SIZE * sizeof( char* ));
    fgets(input, MAX_BUFF_SIZE, stdin);
    //extract tokens from the string input by the user
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
    //getting the program name and compare
    //to decide with program to look for and run
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
        //search the program through all the paths defined in the profile file
        int i = 0;
        for (i = 0; i < MAX_BUFF_SIZE; i++) {
            char buffer[MAX_BUFF_SIZE] = "";
            strcpy(buffer, paths[i]);
            strcat(buffer, "/");
            strcat(buffer, command[0]);
            //if program found run the program with all the arguments
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
        //get the current working directory
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
    //print error message if the profile is not found
    if (loadConfigFile() == false) {
        printf("profile does not exist or if either variable is not assigned");
        return 1;
    }
    else {
        if (chdir(home) == 0) {
            runShell();
        }
        else {
            printf("profile home path does not exist: need change profile home to a valid path\n");
            return 1;
        }
    }
  return 1;
}
