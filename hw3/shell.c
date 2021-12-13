#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define UNDEFINED 0 /* undefined */
#define FOREGROUNG 1    /* running in foreground */
#define BACKGROUND 2    /* running in background */
#define STOPPED 3    /* stopped */


#define MAXCOM 1024 // max number of letters to be supported
#define MAXLIST 128 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

int errno = 0;

typedef struct job{
    int jobID;
    pid_t pid;
    int status;
    char jobname[MAXCOM];
}job;

static job* jobs[MAXLIST];
static int recentJobID;
static int recentPid;

int getNextJobID(){
    for (int i = 0; i < MAXLIST; i++){
        if (jobs[i] == NULL){
            return i;
        }
    }
    return -1;
}

int addJob(pid_t pid, int status, char* jobname){
    struct job* tempJob = malloc(sizeof(struct job));
    int jobID = getNextJobID();
    tempJob->jobID = jobID;
    tempJob->pid = pid;
    tempJob->status = status;
    strcpy(tempJob->jobname, jobname);

    jobs[jobID] = tempJob;

    return tempJob->jobID;
}

void deleteJob(pid_t pid){
    for (int i = 0; i < MAXLIST; i++){
        if (jobs[i]->pid == pid){
            free(jobs[i]);
            jobs[i] = NULL;
            return;
        }
    }
}

void listJobs(){
    for (int i = 0; i < getNextJobID(jobs); i++){
        // if (jobs[i] == NULL) return;
        printf("[%d] %d", jobs[i]->jobID, jobs[i]->pid);
        switch (jobs[i]->status){
        case 0: printf(" Undefined");
            break;
        case 1: printf(" FG Running");
            break;
        case 2: printf(" Running");
            break;
        case 3: printf(" Stopped");
            break;
        }
        printf(" %s\n", jobs[i]->jobname);
    }
    return;
}


void sigint_handler(int sig){
    //printf("ctrl-c for sigint\n");
    return;
}

void sigchld_handler(int sig){
    sigset_t mask_all, prev_all;
    pid_t pid = getpid();

    sigemptyset(&mask_all);
    while (waitpid(pid, NULL, 0) > 0){
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        deleteJob(pid);
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
    return;
}

// Function to take input
int takeInput(char* str){
    char* buf;

    buf = readline(">");
    if (buf != NULL){
        add_history(buf);
        strcpy(str, buf);
        free(buf);
        return 0;
    } else{
        free(buf);
        return 1;
    }
}

void cd(char** parsed){
    char* home = getenv("HOME");
    char cwd[MAXCOM];
    if (parsed[1] == NULL){
        chdir(home);
    } else{
        if (chdir(parsed[1]) == -1){
            perror(parsed[1]);
        }
    }
    getcwd(cwd, sizeof(cwd));
    setenv("PWD", cwd, 1);
}

void fg(char** parsed){
    int jobID;
    if (parsed[1][0] == '%'){
        jobID = atoi(parsed[1]);
    }
    pid_t pid = jobs[jobID]->pid;
    jobs[jobID]->status = FOREGROUNG;
    sigset_t mask_all, prev_all;
    sigemptyset(&mask_all);
    while ((pid = waitpid(-1, NULL, 0)) > 0){
        sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
        deleteJob(pid);
        sigprocmask(SIG_SETMASK, &prev_all, NULL);
    }
}

void bg(char** parsed){
    int jobID;
    if (parsed[1][0] == '%'){
        jobID = atoi(parsed[1]);
    }
    pid_t pid = jobs[jobID]->pid;

    jobs[jobID]->status = BACKGROUND;
    kill(pid, SIGCONT);
}

void killProc(char** parsed){
    int jobID;
    if (parsed[1][0] == '%'){
        jobID = atoi(parsed[1]);
    }
    pid_t pid = jobs[jobID]->pid;
    printf("jobID to be killed: %d\n", pid);

}


void bgExecArgs(char** parsed){
    char cmdName[MAXCOM];
    strcpy(cmdName, parsed[0]);
    int i = 1;
    while (parsed[i] != NULL){
        strcat(cmdName, " ");
        strcat(cmdName, parsed[i]);
        if (strcmp(parsed[i], "&") == 0) parsed[i] = NULL;
        i++;
    }

    // Forking a child
    pid_t pid = fork();
    recentPid = pid;

    // signal(SIGINT, sigint_handler);   /* ctrl-c */
    // signal(SIGTSTP, sigtstp_handler);   /* ctrl-z */
    // signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    sigset_t mask_all, mask_one, prev_one;
    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);
    // signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */


    sigprocmask(SIG_BLOCK, &mask_all, &prev_one);
    if (pid == -1){
        fprintf(stderr, "fork error: %s\n", strerror(errno));
        exit(0);
    } else if (pid == 0){
        sigprocmask(SIG_SETMASK, &prev_one, NULL);
        if (execvp(parsed[0], parsed) < 0){
            printf("%s: command not found\n", parsed[0]);
        }
        exit(0);
    } else{

        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        recentJobID = addJob(pid, BACKGROUND, cmdName);
        sigprocmask(SIG_SETMASK, &prev_one, NULL);
        return;
    }
}

// Function where the system command is executed
void execArgs(char** parsed){
    char cmdName[MAXCOM];
    strcpy(cmdName, parsed[0]);
    int i = 1;
    int status;
    while (parsed[i] != NULL){
        strcat(cmdName, " ");
        strcat(cmdName, parsed[i]);
        i++;
    }

    // Forking a child
    pid_t pid = fork();
    recentPid = pid;

    sigset_t mask_all, mask_one, prev_one;
    sigfillset(&mask_all);
    sigemptyset(&mask_one);
    sigaddset(&mask_one, SIGCHLD);

    sigprocmask(SIG_BLOCK, &mask_all, &prev_one);
    if (pid == -1){
        fprintf(stderr, "fork error: %s\n", strerror(errno));
        exit(0);
    } else if (pid == 0){
        sigprocmask(SIG_SETMASK, &prev_one, NULL);
        if (execvp(parsed[0], parsed) < 0){
            printf("%s: command not found\n", parsed[0]);
            return;
        }
        exit(0);
    } else{
        //printf("JobID: %d, PID: %d\n", jobs[0]->jobID, jobs[0]->pid);
        // waiting for child to terminate
        sigprocmask(SIG_BLOCK, &mask_all, NULL);
        recentJobID = addJob(pid, BACKGROUND, cmdName);
        sigprocmask(SIG_SETMASK, &prev_one, NULL);

        waitpid(pid, &status, 0);
        deleteJob(pid);

        return;
    }
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed){
    int switchOwnArg = 0;
    char* ListOfOwnCmds[6];
    //char* username;

    ListOfOwnCmds[0] = "bg";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "exit";
    ListOfOwnCmds[3] = "fg";
    ListOfOwnCmds[4] = "jobs";
    ListOfOwnCmds[5] = "kill";

    for (int i = 0; i < 6; i++){
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0){
            switchOwnArg = i + 1;
            break;
        }
    }

    switch (switchOwnArg){
    case 1:
        bg(parsed);
        return 1;
    case 2:
        cd(parsed);
        return 1;
    case 3:
        exit(0);
    case 4:
        fg(parsed);
        return 1;
    case 5:
        listJobs(jobs);
        return 1;
    case 6:
        killProc(parsed);
    default:
        break;
    }

    return 0;
}

// function for parsing command words
void parseSpace(char* str, char** parsed, int* cmdCounter){
    int i;
    for (i = 0; i < MAXLIST; i++){
        parsed[i] = strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
    *cmdCounter = i;
}

int main(){
    char inputString[MAXCOM], * parsedArgs[MAXLIST];
    int execFlag = 0, cmdCounter = 0;
    for (int i = 0; i < MAXLIST; i++){
        jobs[i] = NULL;
    }

    clear();

    // signal(SIGINT, SIG_IGN);   /* ctrl-c */
    // signal(SIGTSTP, SIG_IGN);   /* ctrl-z */

    signal(SIGINT, sigint_handler);   /* ctrl-c */
    signal(SIGTSTP, SIG_IGN); /* ctrl-z */
    signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    while (1){

        // print shell line
        // printDir();
        // take input
        if (takeInput(inputString))
            continue;

        parseSpace(inputString, parsedArgs, &cmdCounter);
        execFlag = ownCmdHandler(parsedArgs);
        // execflag returns 0 if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command

        // execute
        if (execFlag == 0){
            if (strcmp(parsedArgs[cmdCounter - 1], "&") == 0){
                bgExecArgs(parsedArgs);
            } else{
                execArgs(parsedArgs);
            }
        }

    }

    return 0;
}
