/**
 * CS3600, Spring 2013
 * Project 1
 * (c) 2013 Alan Mislove
 *
 */

#define _BSD_SOURCE
#include "3600sh.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "errno.h"

#define USE(x) (x) = (x)


int main(int argc, char*argv[]) {
    // Code which sets stdout to be unbuffered
    // This is necessary for testing; do not change these lines
    USE(argc);
    USE(argv);
    setvbuf(stdout, NULL, _IONBF, 0);
  
    // Main loop that reads a command and executes it
    while (1) {
        
        // You should issue the prompt here
        prompt();
        
        // You should read in the command and execute it here
        char *input = (char *)malloc(1024);
        int args = 0;
        
        //
        args = get_line(stdin, input, 1024);
        if (strcmp(input, "exit") == 0) {
            do_exit();
        }
        
        // Create 2d array to store arguments
        char **arglist = (char **)malloc(1024);
        for (int i = 0; i < abs(args) + 1; i++) {
            arglist[i] = (char *)malloc(1024); // TODO change this so that it isn't stupid
        }
        
        // put the input into the buffers for command and arguments
        organize_args(arglist, input, abs(args));
        
        for (int i = 0; i < args; i++) {
            printf("%s\n", arglist[i]);
        }
        
        // execute our command!
        execute(arglist, 0);
        if (args < 0) {
            prompt();
            do_exit();
        }
        
        // FREE EVERYTHING
        for (int i = 0; i < args; i++) {
            free(arglist[i]);
        }
        free(arglist);
        free(input);
    }

    return 0;
}


// prompt user
void prompt() {
    struct passwd *pass;
    pass = getpwuid(getuid());
    
    //char *user = getlogin();
    char host[1024];
    host[1023] = '\0';
    gethostname(host, 1023);
    // TODO check for gethostname err
    char dir[1024];
    dir[1023] = '\0';
    getcwd(dir, 1023);
    // TODO check for getcwd err
    printf("%s@%s:%s> ", pass->pw_name, host, dir);
}


// gets line of input from stdin and returns number of args
int get_line(FILE *fp, char *buffer, size_t buflen) {
    char *end = buffer + buflen - 1; // Allow space for null terminator
    char *dst = buffer;
    int c;
    int spaces = 0;
    while ((c = getc(fp)) != '\n' && dst < end) {
        // TODO fix spacing
        if (c == ' ') {
            spaces++;
            int sflag = 0;
            while (sflag == 0) {
                int z = getc(fp);
                if (z == EOF) {
                    return spaces + 1;
                }
                else if (z == '\n') {
                    return spaces + 1;
                }
                else if (z == ' ') {
                    // do nothing and let getc progress
                }
                else {
                sflag++;
                ungetc(z, fp);
                }
            }
        }
        *dst++ = c;
        *dst = '\0';
    }
    if (c == '\n') {
        int nflag = 0;
        while (nflag == 0) {
            int d = getc(fp);
            if (d == EOF) {
                spaces *= -1;
                return spaces - 1;
            }
            else if (d == ' '){
                // do nothing and let getc progress
            }
            else if (d == '\n') {
                // do nothing and let getc progress
            }
            else {
                nflag++;
                ungetc(d, fp);
            }
        }
    }
    return spaces + 1;
}


// creates a 2d array of our arguments
int organize_args(char **arglist, char *line, int args) {
    int r = 0;
    int c = 0;
    int i = 0;
    while ((line[i] != '\0') && (line[i] != EOF)) {
        if (line[i] == 92) {
            if (line[i + 1] == ' ') {
                arglist[r][c] = ' ';
                c++;
                arglist[r][c] = '\0';
                i++;
            }
            else if (line[i + 1] == 't') {
                arglist[r][c] = ' ';
                arglist[r + 1][c + 1] = ' ';
                arglist[r + 2][c + 2] = ' ';
                arglist[r + 3][c + 3] = ' ';
                c += 4;
                i++;
            }
            else if (line[i + 1] == 92) {
                arglist[r][c] = 92;
                c++;
                arglist[r][c] = '\0';
                i++;
            }
            else if (line[i + 1] == '&') {
                
            }
            else {
                printf("Error: Unrecognized escape sequence.\n");
                do_exit();
            }
        }
        if ((line[i] != ' ') && (line[i] != '\n')) {
            arglist[r][c] = line[i];
            c++;
            arglist[r][c] = '\0';
        }
        else if (line[i] == '\n') {
            r++;
            arglist[r] = NULL;
            execute(arglist, 1);
            prompt();
            for (int i = 0; i < args; i++) {
                for (unsigned int k = 0; k < sizeof(arglist[i]); k++) {
                    arglist[i][k] = '\0';
                }
            }
            r = 0;
            c = 0;
        }
        else if (line[i] == ' ') {
            r++;
            c = 0;
        }
        i++;
    }
    r++;
    arglist[r] = NULL;
    if (line[i] == EOF) {
        //execute(arglist, 0);
        do_exit();
    }
    return 0;
}

// redirect the input/output
int io_redirection(char **arglist) {
    int old_i = dup(STDIN_FILENO);
    int old_o = dup(STDOUT_FILENO);
    int old_e = dup(STDERR_FILENO);
    
    int fd_i = STDIN_FILENO;
    int fd_o = STDOUT_FILENO;
    int fd_e = STDERR_FILENO;
    
    int flag_i = 0;
    int flag_o = 0;
    int flag_e = 0;
    
    int i = 0;
    int j = 0;
    int len = 0;
    
    char * arg;
    arg = arglist[i];
    while (arglist[len] != NULL) {
        len++;
    }
    len++;
    
    while (arg != NULL) { // while there are more arguments to parse
        if (!strcmp(arg, "<")) { // if we need to redirect stdin
            // if there is invalid redirection syntax
            if (arglist[i + 1] == NULL || !strcasecmp(arglist[i + 1], "<")
                || !strcmp(arglist[i + 1], ">")
                || !strcmp(arglist[i + 1], "2>")
                || !strcmp(arglist[i + 1], "&")) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            // else if there is more invalid redirection syntax
            else if (len - i > 3 && !(!strcmp(arglist[i+2], "<")
                                      || !strcmp(arglist[i+2], ">") || !strcmp(arglist[i+2], "2>")
                                      || !strcmp(arglist[i+2], "&") )) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            if (flag_i) {
                // If second time redirecting input
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            fd_i = open(arglist[i+1], O_RDONLY, 0777); // open the input file
            if (fd_i == -1) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Unable to open redirection file.\n");
                return 1;
            }
            dup2(fd_i, STDIN_FILENO); // set STDIN to input file
            j = i;
            while (j < len - 2) {
                arglist[j] = arglist[j+2]; // modify argument list for further parsing
                j++;
            }
            i--; // update vars for modified list
            len = len - 2;
            flag_i++;
        }
        else if (!strcmp(arg, ">")) { // if we need to redirect stdout
            // if there is invalid redirection syntax
            if (arglist[i+1] == NULL || !strcmp(arglist[i+1], "<")
                || !strcmp(arglist[i+1], ">") || !strcmp(arglist[i+1], "2>")
                || !strcmp(arglist[i+1], "&")) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            // else if there is more invalid redirection syntax
            else if (len - i > 3 && !(!strcmp(arglist[i+2], "<")
                                      || !strcmp(arglist[i+2], ">") || !strcmp(arglist[i+2], "2>")
                                      || !strcmp(arglist[i+2], "&") )) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            if (flag_o) {
                // If second time redirecting output
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            fd_o = open(arglist[i+1], O_RDWR|O_CREAT|O_TRUNC, 0777); // open output file
            if (fd_o == -1) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Unable to open redirection file.\n");
                return 1;
            }
            dup2(fd_o, STDOUT_FILENO);
            j = i;
            while (j < len - 2) {
                arglist[j] = arglist[j+2]; // modify argument list for further parsing
                j++;
            }
            i--; // update vars for modified list
            len = len - 2;
            flag_o++;
        }
        else if (!strcmp(arg, "2>")) { // if we need to redirect stderr
            // if there is invalid redirection syntax
            if (arglist[i+1] == NULL || !strcmp(arglist[i+1], "<")
                || !strcmp(arglist[i+1], ">") || !strcmp(arglist[i+1], "2>")
                || !strcmp(arglist[i+1], "&")) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            // else if there is more invalid redirection syntax
            else if (len - i > 3 && !(!strcmp(arglist[i+2], "<")
                                      || !strcmp(arglist[i+2], ">") || !strcmp(arglist[i+2], "2>")
                                      || !strcmp(arglist[i+2], "&") )) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            if (flag_e) {
                // If second time redirecting error
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Invalid syntax.\n");
                return 1;
            }
            fd_e = open(arglist[i+1], O_RDWR|O_CREAT|O_TRUNC, 0777); // open error file
            if (fd_e == -1) {
                reset_redirection(old_i, old_o, old_e);
                printf("Error: Unable to open redirection file.\n");
                return 1;
            }
            dup2(fd_e, STDERR_FILENO);
            j = i;
            while (j < len - 2) {
                arglist[j] = arglist[j+2]; // modify argument list for further parsing
                j++;
            }
            i--; // update vars for modified list
            len = len - 2;
            flag_e++;
        }
        i++;
        arg = arglist[i];
    }
    return 0;
}

// reset STDIN/STDOUT to allow for non-redirected IO
void reset_redirection(int old_i, int old_o, int old_e) {
    // reset our file descriptors
    dup2(old_i, STDIN_FILENO);
    dup2(old_o, STDOUT_FILENO);
    dup2(old_e, STDERR_FILENO);
}


// execute
int execute(char **arglist, int holdup) {
    io_redirection(arglist);
    pid_t pid;
    /* fork another process */
    pid = fork();
    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) { /* child process */
        int i = 0;
        i = execvp(arglist[0], arglist);
        if (i == -1) {
            if (errno == ENOENT) {
                printf("Error: Command not found.\n");
            }
            else if (errno == EACCES) {
                printf("Error: Permission denied.\n");
            }
            else {
                printf("Error: %s\n", strerror(errno));
            }
        }
        exit(1);
        exit(1);
    }
    else if ((pid > 0) && (holdup == 0)) { /* parent process */
        /* parent will wait for the child */
        int status;
        wait(&status);
    }
    else if ((pid > 0) && (holdup == 1)) {
        int status;
        wait(&status);
    }
    return 0;
}


// Function which exits, printing the necessary message
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}
