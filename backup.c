#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>



// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

typedef struct history_list{
  char his[MAX_INPUT];
  struct history_list *next;
  struct history_list *prev;
} his_list;

//global var
his_list *head;

char prompt[1024];
FILE *fd;
FILE *ff;
char file_path[1024];
char full_his_path[1024];
// clock_t start,end;
int cmd_argc;
int builtin_redirect;
int is_debug = 0;
// double time_used;

int builtin_cmd (char* argv[]);
int parseline(char *buf, char **argv);
void eval(char *cmdline,char *envp[]) ;
void sh_pwd();
void sh_cd(char *argv[]);
void sh_set(char *argv[]);
void sh_echo(char *argv[]);
his_list* new_his(char *his);
void add(his_list* new_his);
void remove_head();
int builtin_cmd_check(char* argv[]);
void sh_history();
void sh_clear_history();

int main (int argc, char ** argv, char **envp) {
  // start = clock();
  int finished = 0;
  int length = 0;
  char temp[1024];
  strcpy(prompt, "[");
  strcat(prompt, getcwd(temp, sizeof(temp)));
  strcat(prompt, "] 320sh> ");
  char cmd[MAX_INPUT];
  char line[MAX_INPUT];
  his_list *pointer;

  if(argv[1] != NULL && !strcmp(argv[1], "-d")){
    is_debug = 1;
  }
  
  //open histroy file
  strcpy(file_path, getenv("HOME"));
  strcat(file_path, "/.320sh_history");
  fd = fopen(file_path, "a+");

  full_his_path[0] = '\0';
  strcat(full_his_path, "320sh_full_history.txt");
  ff = fopen(full_his_path, "a+");  
  
  //cp the content of the file into his_list
  while(fgets(line, sizeof(line), fd)){
    his_list *new_history = new_his(line);
    add(new_history);
    pointer = new_history;
  }
  
  fclose(fd);
  while (!finished) {
    char *cursor;
    char last_char, current;
    int rv;
    int count;
    int counter = 0;
    int back_count;
    //first = 1;
    
    // Print the prompt
    rv = write(1, prompt, strlen(prompt));
    if (!rv) {
      finished = 1;
      break;
    }
    
    
    // read and parse the input
    for(rv = 1, count = 0,
      cursor = cmd, last_char = 1;
      rv
      && (++count < (MAX_INPUT-1))
      && (last_char != '\n');
      cursor++) {
        rv = read(0, &current, 1);
        last_char = current;
        if(last_char == 3) {
          write(1, "^c", 2);
        }
        else if(last_char == '\033') {
          rv = read(0, &current, 1);
          rv = read(0, &current, 1);
          last_char = current;
          char back = '\b';
          switch(last_char) { // the real value
            case 'A':
            //code for arrow up

            //clear buf
            //move the cursor to the end of string
            back_count = strlen(cursor);
            while(back_count>0){
              write(1, " ", 1);
              back_count--;
            }
            back_count = strlen(cmd);
            while(back_count>0){
              write(1, "\b \b", 3);
              back_count--;
            }
            
            
            //print the prev cmd
            write(1, pointer->his, strlen(pointer->his)-1);
            
            
            cmd[0] = '\0';
            strncat(cmd, pointer->his, strlen(pointer->his)-1);//remove '\n'
            cursor = &cmd[strlen(cmd)];

            if(pointer->prev != NULL){
              pointer = pointer->prev;
            }
            

            break;
            
            case 'B':
            //code for arrow down
            // if(ptr != NULL && ptr->next != NULL){
            //   counter = strlen(ptr->his)-1;
            //   while(counter > 0){
            //     //delete the char on cosole
            //     char *temp = "\b \b";
            //     write(1, temp, strlen(temp));
            //     counter--;
            //   }
            //   ptr = ptr->next;
            //   //print the next cmd
            //   write(1, ptr->his, strlen(ptr->his)-1);
            //   cmd[0] = '\0';
            //   strncat(cmd, ptr->his, strlen(ptr->his));//remove '\n'
            //   //cursor = cursor + strlen(cmd);
            //   cursor = &cmd[strlen(cmd)-2];
            // }
             //clear buf
            //move the cursor to the end of string
            back_count = strlen(cursor);
            while(back_count>0){
              write(1, " ", 1);
              back_count--;
            }
            back_count = strlen(cmd);
            while(back_count>0){
              write(1, "\b \b", 3);
              back_count--;
            }
            
            
            //print the prev cmd
            if(pointer->next != NULL){
              pointer = pointer->next;
              write(1, pointer->his, strlen(pointer->his)-1);
            }
            
            cmd[0] = '\0';
            strncat(cmd, pointer->his, strlen(pointer->his)-1);//remove '\n'
            cursor = &cmd[strlen(cmd)];

            break;

            case 'C':
            // code for arrow right
            
            if(cursor != &cmd[strlen(cmd)]){
              write(1, cursor, 1);
            } else {
              cursor--; //prevent cursor to go further
            }
            
            break;
            case 'D':
            // code for arrow left
            if(cursor != &cmd[0]){
              cursor = cursor - 2;
              
              //move cursur on consle
              write(1, &back, 1);
            } else {
              cursor--; //prevent cursor to go further
            }
            
            break;
          }
        }
        
        else if(last_char == 127 || last_char == 8){
          
          if(cursor != &cmd[0]){
            char temp[1024];
            int back_count = 0;
            memset(temp, '\0', 1024);
            strcpy(temp, cursor);
            back_count = strlen(temp);
            cursor--;
            strcpy(cursor, temp);
            
            write(1, "\b", 1);
            write(1, temp, strlen(temp));
            write(1, " ", 1);
            back_count++;
            while(back_count > 0){
              write(1, "\b", 1);
              back_count--;
            }
          }
          cursor--; //one more movement back for the increament inside forloop
        }
        
        
        
        else if(last_char == '\n'){
          cursor = &cmd[strlen(cmd)]; //move cursor to the end of cmd
          strncpy(cursor, &current, 1);
          write(1, &last_char, 1);
        }
        
        else {
          if(cursor == &cmd[strlen(cmd)]){
            write(1, &last_char, 1);
            strncpy(cursor, &current, 1);
            counter++;
          } else {
            char temp[1024];
            int back_count = strlen(cursor);
            memset(temp, '\0', 1024);
            strncpy(temp, &current, 1);
            strcat(temp, cursor);
            strcpy(cursor, temp);
            
            write(1, temp, strlen(temp));
            while(back_count > 0){
              write(1, "\b", 1);
              back_count--;
            }
          }
          
        }
      }
      //cmd[strlen(cmd)-1] = '\0';
      *cursor = '\0';
      //printf("***the cmd is: %s***", cmd);
      
      if (!rv) {
        finished = 1;
        break;
      }
      
      //check the length of the his_list, if longer than 50, delete the head
      if(head != NULL){
        his_list *ptr = head;
        while(ptr->next != NULL){
          length++;
          ptr = ptr->next;
        }
        
        //check whether cmd is same as prev cmd
        if(strcmp(ptr->his, cmd)){
          if(length >= 25){
            remove_head();
          }
          //add the value to doubly linked list
          his_list *new_history = new_his(cmd);
          add(new_history);
          pointer = new_history;
          
          
        }
      } else {
        //add the value to doubly linked list
        his_list *new_history = new_his(cmd);
        add(new_history);
        pointer = new_history;
      }
      
      fputs(cmd, ff);
      //printf("***the cmd is: %s***", cmd);
      eval(cmd,envp);
      // Execute the command, handling built-in commands separately
      // Just echo the command line for now
      // write(1, cmd, strnlen(cmd, MAX_INPUT));
      //emtpy cmd buffer
      memset(cmd, '\0', 1024);
      
    }
    
    return 0;
  }
  
  void eval(char *cmdline,char *envp[])
  {
    char *argv[10]; /* Argument list execve() */
    char buf[MAX_INPUT];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    int file_descriptor[2];
    strcpy(buf, cmdline);
    int index =0;

    cmd_argc = 0;
   // int fd_in =0;

    bg = parseline(buf, argv);
    builtin_redirect = 0;
    int loopcont = 1; // at least once
    // char *pipe_buffer[20][20] ;
    // char *temp_buf[20];


    if (argv[0] == NULL)
    return;   /* Ignore empty lines */
      int i;
    for(i = 0; i < cmd_argc;i++)
    {
      if (!strcmp (argv[i],">"))
      {
        if(index == 0)
        {
          index =i;
        }
        builtin_redirect = 1;
        loopcont++;
      }
      if (!strcmp (argv[i],"<"))
      {
        if(index == 0)
        {
          index =i;
        }
        builtin_redirect = 2;
        loopcont++;
      }
      if (!strcmp (argv[i],"|"))
      {

        builtin_redirect = 3;
        loopcont++;
      }
    }

    // if(builtin_redirect == 3)
    // {
    //
    // }


      if ((pid = fork()) == 0) {
          if (builtin_cmd_check(argv) == 1) {
            int i;
            for( i = 0; i < cmd_argc;i++)
            {
              if (!strcmp (argv[i],">"))
              {
                builtin_redirect =1;
                file_descriptor[0]=open (argv[i+1],O_CREAT|O_WRONLY,0666);
                if(argv[i+1] == argv[cmd_argc-1])
                {
                dup2(file_descriptor[0],1);
                close(file_descriptor[0]);
                if (index != 0)
                {
                  argv[index] = NULL;
                }
                builtin_cmd(argv);
                exit(0);
              }
              }

              if (!strcmp (argv[i],"<"))
              {
                builtin_redirect = 2;

                file_descriptor[1] = open(argv[i+1],O_RDONLY| O_RDWR| O_APPEND);
                if(argv[i+1] == argv[cmd_argc-1])
                {
                dup2(file_descriptor[1],0);
                close(file_descriptor[1]);
                if (index != 0)
                {
                  argv[index] = NULL;
                }

                builtin_cmd(argv);
                exit(0);
              }
              }

            }

            exit(0);

          }
        else{

        char temp[1024];
        char *path;

        strcpy( temp,getenv("PATH"));
        path = strtok(temp,":");
        struct stat st;
        while( path != NULL )
        {
          char path_modify[1024];
          strcpy(path_modify, path);
          strcat(path_modify,"/");
          strcat(path_modify, argv[0]);

          if(stat(path_modify,&st)== 0)
          {
            int i;
            for(i = 0; i < cmd_argc;i++)
            {
              if (strcmp (argv[i],">") == 0)
              {

                file_descriptor[0]=open (argv[i+1],O_CREAT|O_WRONLY,0666);
                if(argv[i+1] == argv[cmd_argc-1])
                {
                dup2(file_descriptor[0],1);
                close(file_descriptor[0]);
                if (index != 0)
                {
                  argv[index] = NULL;
                }

                execve(path_modify, argv, envp);
                }
              }

              if (!strcmp (argv[i],"<"))
              {
                file_descriptor[1] = open(argv[i+1],O_RDONLY| O_RDWR| O_APPEND);
                if(argv[i+1] == argv[cmd_argc-1])
                {
                dup2(file_descriptor[1],0);
                close(file_descriptor[1]);
                if (index != 0)
                {
                  argv[index] = NULL;
                }
                execve(path_modify, argv, envp);
                }
              }


            }


            // if(builtin_redirect ==3)
            // {
            //   strcpy(buf, cmdline);
            //   char *tok;
            //   tok = strtok(buf,"|");
            //   int a =0;
            //   pid_t pid2;
            //   while(tok != NULL)
            //   {
            //     temp_buf[a] =tok;
            //     a++;
            //     tok = strtok(NULL,"|");
            //   }
            //
            //   a = 0;
            //   while (temp_buf[a] != NULL)
            //   {
            //     parseline(temp_buf[a],pipe_buffer[a]);
            //     a++;
            //   }
            //   // pipe_buffer[a] = NU
            //
            //
            //   int i =0;
            //   while (i < a)
            //   {
            //     pipe(file_descriptor);
            //     if((pid2 = fork()) == -1)
            //     {
            //       fprintf(stderr, "pipe failed\n" );
            //       exit(1);
            //     }
            //     else if(pid2 == 0)
            //     {
            //       //child
            //       if(i != 0)
            //       {
            //         dup2(fd_in,0);
            //         close(fd_in);
            //         fd_in = file_descriptor[0];
            //       }
            //         if(pipe_buffer[i+1]!= NULL)
            //         {
            //           dup2(file_descriptor[1],1);
            //           close(file_descriptor[1]);
            //         }
            //
            //         execvp((pipe_buffer)[i][0],pipe_buffer[i]);
            //     }
            //     else
            //     {
            //       //parent
            //       wait(NULL); // wait for child terminate
            //       close(fd_in);
            //       if(pipe_buffer[i+1]!= NULL)
            //       {
            //         close(file_descriptor[1]);
            //       }
            //
            //       fd_in = file_descriptor[0];
            //
            //
            //       i++;
            //     }
            //   }
            // }
            if(builtin_redirect == 0)
            {
              execve(path_modify, argv, envp);
            }
            exit(0);
          }

          path = strtok(NULL, ":");
        }
        printf("%s: Command not found.\n", argv[0]);
        exit(127);
      }

      }



      /* Parent waits for foreground job to terminate */

      if (!bg) {
        int status;
        if (waitpid(pid, &status, 0) < 0){
          fprintf(stderr, "%s: %s\n", "waitfg: waitpid error", strerror(errno));
        }


        if( builtin_cmd_check(argv)==1 && builtin_redirect == 0)
        {
          printf("built-in without redirect\n");
          builtin_cmd(argv);
        }
        // end = clock();
        // time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

        // printf("TIMES: %f",time_used);  

        return;
      }
      else
      printf("%d %s", pid, cmdline);


    return;
  }
  
  int parseline(char *buf, char **argv)
  {
    char *space;         
    int argc = 0;            
    int bg;              
    
    buf[strlen(buf)-1] = ' ';  //ignore \n
    //ingore empty 
    while (*buf && !strncmp(buf, " ", 1)){
      buf++;
    } 
    
    //store to argv
    while ((space = strchr(buf, ' '))) {
      argv[argc] = buf;
      argc++;
      *space = '\0';
      buf = space + 1;
      //ingore empty 
      while (*buf && !strncmp(buf, " ", 1)){
        buf++;
      } 

      //detect the "
      if(!strncmp(buf, "\"", 1)){
        buf++;
        argv[argc] = buf;
        argc++;
        while(*buf != '\"'){
          buf++;
        }
        *buf = '\0';
        buf++;
        //ingore empty 
        while (*buf && !strncmp(buf, " ", 1)){
          buf++;
        } 
      }

    }
    argv[argc] = NULL;
    
    if (argc == 0){
      return 1;
    }
    
    //detect bg symbol
    if ((bg = (*argv[argc-1] == '&')) != 0){
       argv[--argc] = NULL; 
    } 
    
    cmd_argc = argc;

    return bg;
  }
  
  int builtin_cmd_check(char* argv[])
  {
    if(!strcmp(argv[0],"exit"))
    {
      return 1;
    }
    else if(!strcmp(argv[0],"&")){
      return 1;
    }
    else if(!strcmp(argv[0], "pwd")){
      return 1;
    }
    else if(!strcmp(argv[0], "cd")){
      
      return 1;
    }
    else if(!strcmp(argv[0], "set")){
      
      return 1;
    }
    else if(!strcmp(argv[0], "echo")){
      
      return 1;
    }

    else if(!strcmp(argv[0], "history")){
      return 1;
    }
    else if(!strcmp(argv[0], "clear-history")){
      return 1;
    }
    else {
      return 0;
    }
  }
  
  int builtin_cmd (char* argv[]){
    if(!strcmp(argv[0],"exit")){
      fd = fopen(file_path, "w");
      his_list *cursor = head;
      //write the history list into his file
      while(cursor->next != NULL){
        fputs(cursor->his, fd);
        cursor = cursor->next;
      }
      fclose(fd);

      fclose(ff);

      exit (0);
    }
    if(!strcmp(argv[0],"&")){
      return 1;
    }
    if(!strcmp(argv[0], "pwd")){
      sh_pwd();
      return 1;
    }
    if(!strcmp(argv[0], "cd")){
      sh_cd(argv);
      return 1;
    }
    if(!strcmp(argv[0], "set")){
      sh_set(argv);
      return 1;
    }
    if(!strcmp(argv[0], "echo")){
      sh_echo(argv);
      return 1;
    }
    if(!strcmp(argv[0], "history")){
      sh_history();
      return 1;
    }
    if(!strcmp(argv[0], "clear-history")){
      sh_clear_history();
      return 1;
    }
    return 0;
  }
  
  void sh_history(){
    char line[MAX_INPUT];
    fclose(ff);
    ff = fopen(full_his_path, "a+"); 
    while(fgets(line, sizeof(line), ff)!= NULL){
      printf("%s", line);
    }
  }

  void sh_clear_history(){
    fclose(ff);
    ff = fopen(full_his_path, "w+");
  }

  void sh_pwd(){
    printf("%s\n", getenv("PWD"));
  }
  
  void sh_cd(char *argv[]){
    char buffer[1024];
    char prev[1024];
    /* cd */
    if(argv[1] == NULL){
      strcpy(prev, getenv("PWD"));
      chdir(getenv("HOME"));
      setenv("PWD", getenv("HOME"), 1);
      setenv("OLDPWD", prev, 1);
    }
    
    /* cd - */
    else if(!strcmp(argv[1], "-")){
      strcpy(prev, getenv("PWD"));
      chdir(getenv("OLDPWD"));
      setenv("PWD", getenv("OLDPWD"), 1);
      setenv("OLDPWD", prev, 1);
    }
    
    /* cd .. */
    else if(!strcmp(argv[1], "..")){
      strcpy(prev, getenv("PWD"));
      chdir("..");
      setenv("PWD", getcwd(buffer, sizeof(buffer)), 1);
      setenv("OLDPWD", prev, 1);
    }
    
    /* cd . */
    else if(!strcmp(argv[1], ".")){
      strcpy(prev, getenv("PWD"));
      setenv("PWD", getcwd(buffer, sizeof(buffer)), 1);
      setenv("OLDPWD", prev, 1);
    }
    
    /* cd directory */
    else{
      strcpy(prev, getenv("PWD"));
      //check whether this is an absolute path
      if(!strncmp(argv[1], "/", 1)){
        if(chdir(argv[1]) == 0){ //success
          setenv("PWD", getcwd(buffer, sizeof(buffer)), 1);
          setenv("OLDPWD", prev, 1);
        }
        else {
          fprintf(stderr, "%s\n", strerror(errno));
        }
      }
      
      else {
        strcpy(buffer, prev);
        strcat(buffer, "/");
        strcat(buffer, argv[1]);
        if(chdir(buffer) == 0){ //success
          setenv("PWD", getcwd(buffer, sizeof(buffer)), 1);
          setenv("OLDPWD", prev, 1);
        }
        else {
          fprintf(stderr, "%s\n", strerror(errno));
        }
      }
      
    }
    
    strcpy(prompt, "[");
    strcat(prompt, getcwd(buffer, sizeof(buffer)));
    strcat(prompt, "] 320sh> ");
  }
  
  void sh_set(char* argv[]){
    char buffer[1024];
    strcpy(buffer,"");
    int i = 1;
    while(argv[i] != NULL){
      strcat(buffer, argv[i]);
      i++;
    }
    char *variable;
    char *value;
    variable = strtok(buffer,"=");
    value = strtok (NULL,"=");
    setenv(variable,value,1);
    
  }
  
  void sh_echo(char *argv[]){
    char var[1024];
    if(!strncmp(argv[1], "$", 1)){
      strcpy(var, &argv[1][1]);
      printf("%s\n", getenv(var));
    }
  }
  
  his_list* new_his(char *his){
    his_list* new_his = (his_list *)malloc(sizeof(his_list));
    strcpy(new_his->his, his);
    new_his->prev = NULL;
    new_his->next = NULL;
    return new_his;
  }
  
  void add(his_list* new_his){
    his_list *cursor = head;
    if(head == NULL) {
      head = new_his;
      return;
    }
    while(cursor->next != NULL){
      cursor = cursor->next;
    }
    cursor->next = new_his;
    new_his->prev = cursor;
    new_his->next = NULL;
  }
  
  void remove_head(){
    head = head->next;
    head->prev = NULL;
  }
