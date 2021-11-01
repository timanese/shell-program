// Tim Yang
// COP 4600 - OS
// Shell program called Mysh
// - it prints a prints a prompt "#" and reads a command line terminated
// by a new line. This line should be parsed out into a command and all its arguments.
// only supported delimiter is whitespace
// no need to angle special characters
// should handle any length of input lines.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
// For pid_t useage
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


#define FILENAMEMAX 100 // max length of filename
#define MAXLIST 100 // number of command line arguments allowed
#define MAXCHAR 1000 // max number of letters to be supported

void moveToDir(char**);
void whereAmI(void);
void history(char**);
void byeBye(void);



typedef struct node
{
    char *command;
    struct node *next;
} node;

typedef struct LinkedList
{
    node *head;
    node *tail;
} LinkedList;

// global variable
char currentDir[FILENAMEMAX];
struct LinkedList *historyList;
FILE * ifp;

LinkedList *createList(void)
{
    LinkedList *newList = malloc(sizeof(LinkedList));

    newList->head = NULL;
    newList->tail = NULL;

    return newList;
}

node *createNode(char *command)
{
    node *newNode = malloc(sizeof(node));

    newNode->command = command;
    newNode->next = NULL;
    return newNode;
}


void tailInsert(LinkedList *list, char *command)
{
    if (list == NULL)
    {
        return;
    }
    else if(list->tail == NULL)
    {
        list->head = list->tail = createNode(command);
    }
    else
    {
        list->tail->next = createNode(command);
        list->tail = list->tail->next;

    }
}



void print_list(node *head)
{
    if (head == NULL)
    return;

    print_list(head->next);
    printf(" %d %s\n", head->command);

}



node *destroy_list(node *head)
{
    if (head == NULL)
        return NULL;
    destroy_list(head->next);
    free(head);
    return NULL;
}
LinkedList *destroy_linked_list(LinkedList *list)
{
    if (list == NULL)
        return NULL;
    // Free the entire list within this struct.
    destroy_list(list->head);
    // Free the struct itself.
    free(list);
    return NULL;
}


char *takeInput(void)
{
    int bufsize = MAXCHAR;
     int position = 0;
     char *buffer = malloc(sizeof(char) * bufsize);
     int c;

     if (!buffer) {
       fprintf(stderr, "allocation error\n");
       exit(EXIT_FAILURE);
     }

     while (1) {
       // Read a character
       c = getchar();

       // If we hit EOF, replace it with a null character and return.
       if (c == EOF || c == '\n') {
         buffer[position] = '\0';
         return buffer;
       } else {
         buffer[position] = c;
       }
       position++;

       // If we have exceeded the buffer, reallocate.
       if (position >= bufsize) {
         bufsize += MAXCHAR;
         buffer = realloc(buffer, bufsize);
         if (!buffer) {
           fprintf(stderr, "allocation error\n");
           exit(EXIT_FAILURE);
         }
       }
     }
}

char **tokenizer(char *line)
{
    int buffSize = MAXLIST;
    int position = 0;
    char **tokens = malloc(MAXLIST * sizeof(char*));
    char *tokenBuf;

    // Storing input from function takeInput and putting it into
    // why didnt i have to allocate it twice ? because its a double hmm. tokens
    tokenBuf = strtok(line," \t\r\n\a");
    while(tokenBuf != NULL)
    {
        tokens[position] = tokenBuf;
        position++;
        if(position >= buffSize)
        {
            buffSize += MAXLIST;
            tokens = realloc(tokens, buffSize * sizeof(char*));

        }
        tokenBuf = strtok(NULL, " \t\r\n\a");
    }

    tokens[position] = NULL;
    return tokens;

}

int launch(char ** args)
{
    pid_t pid;
    pid_t wpid;
    int status;
    pid = fork();

    if (pid == 0)
    {
      // Child process
      if (execvp(args[0], args) == -1)
      {
        perror("mysh");
      }
      exit(EXIT_FAILURE);
    } else if (pid < 0)
    {
      // Error forking
      perror("mysh");
    }
    else
    {
      // Parent process
      do
      {
        wpid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int execute(char **args)
{

  int numOfCmd = 8, switchNum = 0;
  char *ListOfCmd[numOfCmd];
  int i;

  ListOfCmd[0] = "movetodir";
  ListOfCmd[1] = "whereami";
  ListOfCmd[2] = "history";
  ListOfCmd[3] = "byebye";
  ListOfCmd[4] = "replay";
  ListOfCmd[5] = "start";
  ListOfCmd[6] = "background";
  ListOfCmd[7] ="dalek";
  // if no input is entered
  if (args[0] == NULL)
  {
      return 1;
  }
  for (i = 0; i < numOfCmd; i++)
  {
      // arg 0 because that is the one that will contain the command i.e cd or ls
      if (strcmp(args[0], ListOfCmd[i]) == 0)
      {
          // if it is the matching command then break and continue to the switch
          switchNum = i+1;
          break;
      }
  }
  printf("here\n");
  switch (switchNum)
  {
      case 1:
        moveToDir(args);
        return 1;
      case 2:
        whereAmI();
        return 1;
      case 3:
        history(args);
        return 1;
      case 4:
        byeBye();
        return 1;

    default:
        break;

  }

  return launch(args);

}

void shellLoop(void)
{
    int historyCnt = 0;
    ifp = NULL;
    historyList = createList();
    char *line;
    char *historyBuffer;
    char **args;
    int status;
    do
    {
        printf("# ");
        // take in input / read in line
        line = takeInput();
        ifp = fopen("historylog.txt", "w");
        if (strlen(line) != 0)
        {
            //put command into history
            if(ifp == NULL)
            {
                printf("Error opening history.txt");
            }

            fprintf(ifp, "%s\n",line);
        }
        // if no command is entered dont add it to history
        if (strcmp(line, "") != 0)
        {
            tailInsert(historyList,line);
        }
        // parse the line of code into tokens
        args = tokenizer(line);


        // Execute - start the shell process
        status = execute(args);




    } while(status);
}

int main(int argc, char **argv)
{

    // getting currentDir
    char dirBuff[FILENAMEMAX];
    getcwd(dirBuff, FILENAMEMAX);
    strcpy(currentDir,dirBuff);

    shellLoop();

    return 0;
}
// look into opendir later
void moveToDir(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "%s\n","Too few arguments." );
        return;
    }
    struct stat info;
    char path[FILENAMEMAX] = "";

    if(args[1][0] != '/')
    {

        strcat(currentDir,"/");

        strcpy(path, currentDir);
    }

    strcat(path,args[1]);

    if (stat(path,&info) != 0)
    {
        printf("Directory does not exist. \n");
    }
    else if (info.st_mode & S_IFDIR)
    {
        // make sure what they want just the file or the file path
        strcpy(currentDir,path);

        if (currentDir[strlen(currentDir) - 1] == '/')
        {
            currentDir[strlen(currentDir) - 1] = '\0';
        }
    }
    else
    {
        printf("Directory does not exist. \n");
    }

}

void whereAmI(void)
{
    printf("current Directory: %s\n",currentDir);
}

void history(char **args)
{

    if(args[1] == NULL)
    {
        printf("----Here is the history:----\n");
        print_list(historyList->head);
        return;
    }
    if(args[1] != NULL)
    {
        if (strcmp(args[1], "-c") == 0)
        {
            destroy_linked_list(historyList);
            historyList = createList();
            return;
        }
    }
    else
    {
        printf("Incorrect parameters.\n");
        return;
    }

}

void byeBye(void)
{
    fclose(ifp);
    exit(0);
}

void replay(int num)
{
    return;
}

void startProgram(void)
{
    return;
}

void backGroundProgram(void)
{
    return;
}

void dalekPid(void)
{
    return;
}
