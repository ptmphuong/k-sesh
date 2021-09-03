#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#define MAXARGS 10
#define MAXLINE 8192

// TODO: Put your command names here
char* commands[] = {"game", "help", "cd", "exit"};
char* cd_commands[] = {"cd"};
char* exit_commands[] = {"exit"};
char* help_commands[] = {"help"}; 
char* game_commands[] = {"game"}; 


/***
 **  Wrapper of fgets. Checks and quits on Unix error. 
 **/
char* Fgets(char *ptr, int n, FILE *stream) {
  char *rptr;
  if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream)) {
    fprintf(stderr, "%s\n", "fgets error");
    exit(0);
  }
  return rptr;
}

int Cd(char** argv) {
  char path[1000];
  strcpy(path, argv[1]);

  char cwd[1000];
  if(argv[1][0] != '/') {
     getcwd(cwd, sizeof(cwd));
     strcat(cwd, "/");
     strcat(cwd, path);
     chdir(cwd);
  } else {
     chdir(argv[1]);
}
}

int Help(char** argv) {
  // TODO: Implement cd
  printf("  cd: change directory.\n  help: see a list of all command.\n  game: play a guessing game.\n  exit: exit the program.\n");

  return 0;
}

int Game(char** argv) {
  printf("Guess a random number from 1-100\n");
  int high = 100;
  int low = 0;
  int result = (low + (rand() % (high-low)));
  char* guess;
  int guess_num;

  do {
    Fgets(guess, MAXLINE, stdin);
    guess_num = atoi(guess); 

    if (guess_num < result) {
      low = guess_num;
    } else if (guess_num > result) {
      high = guess_num;
    }


    printf("Try again. Number is between %d - %d\n", low, high);
  } while (guess_num != result);
  printf("Bingo. The number is %d\n", result);
  return 0;
}

// TODO: Put your command function pointers here
int (*command_functions[])(char**) = {&Game, &Help, &Cd};

/***
 **  Wrapper of fork(). Checks for fork errors, quits on error. 
 **/
pid_t Fork(void) {
  pid_t pid;  
  if ((pid = fork()) < 0) {
    fprintf(stderr, "%s: %s\n", "fork error", strerror(errno));
    exit(0);
  }
  return pid;
}

// Is the command one built into the shell?
// That is, that the *shell* has implemented?
// If so, execute it here
//
/*
int builtin_command(char** argv) {
  // Loop through
  int i;
  for (i = 0; i < 6; i++) {
    int res = strcmp(commands[i], argv[0]);
 
    if (res == 0) {
    // call the right function.
      (command_functions[i])(argv[1]); // some argument
    }
  }
}
*/
int in_builtin_command(char** argv, char** command_list) {

  int i;
  if (argv[0] == NULL) return 1;

  int result = 1;
  for (i = 0; i < 10; i++) {
    if(!strcmp(command_list[i], argv[0])) {
      return 0;
    }
  }
  return result;
}

char** split_args(char* cmdline) {
  char** argv = malloc(MAXLINE * sizeof(char*));
  char buf[MAXLINE]; /* Holds modified command line */
  int index = 0;

  strcpy(buf, cmdline);
  // Split buf into args and put into argv
  char* token = strtok(buf, " ");
  while (token != NULL) {
     int token_len = strlen(token);
     //token[token_len] = '\0';
     argv[index] = token;
     printf("argv %d: %s\n", index, argv[index]);
     index++;
     token = strtok(NULL, " ");
  }
  
  // printf("test in buildin command %d\n", in_builtin_command(argv));

  if (argv[0] == NULL) {
    printf("empty\n");
    return; /* Ignore empty lines */
  }

  int argv0_len = strlen(argv[0]);
  if (argv[0][argv0_len - 1] == '\n') {
      argv[0][argv0_len - 1] = '\0';
  }

  return argv;
}

int eval(char** argv) {
  pid_t pid; /* Process id */
  pid_t waitPid; /* wait process id */
  int status;

// Check that the command/program exists in Unix (ie /bin/) OR (see below) 

  if (in_builtin_command(argv, commands) == 1) {
    printf("not builtin command\n");

    // Create a child process 
    pid = fork();

    // running child process
    if (pid == 0) {
      if (execvp(argv[0], argv) == -1) { 
        perror("k-sea shell error");
      }
      exit(EXIT_FAILURE);

    } else if (pid < 0) { // error forking
        perror("k-sea shell error");

    } else { // running parent process
      do {
        waitPid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // exit normally (or with an error code), or killed by a signal
    }


    // Run the program/command (execve...)-- what is the result of exec if the command is bogus?
//  char* argv[] = { "jim", "jams", NULL };
  char* envp[] = { "some", "environment", NULL };
    int res = execve(".", argv, envp); // see run_ls.c for an example
    if (res < 0) {
      //print error message
    }

  } else {
    //printf("is builtin command\n");

    if (in_builtin_command(argv, game_commands) == 0) {
      Game(argv);
    } else if (in_builtin_command(argv, help_commands) == 0) {
      Help(argv);
    } else if (in_builtin_command(argv, exit_commands) == 0) {
      return 1;
    } else if (in_builtin_command(argv, cd_commands) == 0) {
        if (argv[1] == NULL) {
           printf("must include a path\n");
           return 0;
        }
        Cd(argv);
     }

  // TODO: Be sure to wait for the child process to terminate
  
   }
  return 0;
}

int main() {

  char cmdline[MAXLINE]; /* command line buffer */
  char** argv;

  while (1) {
    // Print command prompt
    printf("> ");

    // Read input from user
    Fgets(cmdline, MAXLINE, stdin);
    argv = split_args(cmdline);
    
    // If we get the eof, quit the program/shell
    if (feof(stdin))
      exit(0);

    // Otherwise, evaluate the input and execute. 
    int eval_status = eval(argv);
    if (eval_status == 1) {
      break;
    }

    free(argv);

  }
}
