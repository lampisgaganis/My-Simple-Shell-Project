#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <list>
#include <iterator>
#include <glob.h>
#include <map>
#include <signal.h>
#define MAX_INPUT 1024
#define MAX_TOKEN 100
#define MAX_ARG   100

using namespace std;

map<char *,char **> aliases;

void handle_signal(int sig)
{
    cout << endl; //signal handler for parent process SIGTSPT, SIGINT, essentially just ignores the signal, signal is automatically passed to child processes
}

void io_redirection(char * command_arguments[], int size)
{
    char * command[MAX_ARG];
    int j = 0, io_file_descriptor;
    for(int i = 0; i < size; i++)
    {
        if(strcmp(command_arguments[i],">") == 0 && strcmp(command_arguments[i+1],">") != 0 ) // token equal to >
        {
            i++;
            io_file_descriptor = open(command_arguments[i], O_WRONLY | O_TRUNC | O_CREAT, 0666);
            close(1);
            dup2(io_file_descriptor, STDOUT_FILENO);
            close(io_file_descriptor);
        }
        else if(strcmp(command_arguments[i],"<") == 0) // token equal to <
        {
            i++;
            io_file_descriptor = open(command_arguments[i], O_RDONLY);
            close(0);
            dup2(io_file_descriptor, STDIN_FILENO);
            close(io_file_descriptor);
        }
        else if(strcmp(command_arguments[i],">") == 0 && strcmp(command_arguments[i+1],">") == 0) //token equal to >>
        {
            i+=2;
            io_file_descriptor = open(command_arguments[i], O_WRONLY | O_APPEND);
            close(1);
            dup2(io_file_descriptor, STDOUT_FILENO);
            close(io_file_descriptor);
        }
        else
        {
            command[j++] = command_arguments[i];
        }
    }
    command[j] = NULL;
    if(strcmp(command[0],"createalias") == 0)
    {
        char * alias_name = command[1];
        char ** alias_command = command + 2;
        aliases.insert({alias_name, alias_command});
        cout << "Created alias " << alias_name << endl;
        char ** command_alias = aliases[command[1]];
        cout << "Executing alias " << command_alias[0] << endl;
        execvp(command_alias[0], command_alias);
    }
    else if(aliases.count(command[0]))//count > 0 == command exists in aliases (couldnt get this part to work :( )
    {
        char ** command_alias = aliases[command[0]];
        cout << "Executing alias" << command_alias[0] << endl;
        execvp(command_alias[0], command_alias);
    }
    else
    {
        execvp(command[0], command);
        printf("Command %s not found\n", command[0]);
        exit(1);
    }
}

void pipe_commands(char * commands[], int token_amount)
{
    int is_foreground_pipeline = 1; //flag for background execution
    if(strcmp(commands[token_amount-1],"&") == 0)
    {
        is_foreground_pipeline = 0;
        token_amount--;
    }
    int command_count = 1; //counter for amount of child processes needed
    for(int i = 0; i < token_amount; i++)
    {
        if(strcmp(commands[i],"|") == 0) // for every pipe add one more process
        {
            command_count++;
        }
    }
    int pipe_file_descriptors[2];
    int input_file_descriptor = 0, output_file_descriptor;
    list<int> pids; // list for child process process ids
    for(int i = 0; i < command_count; i++)
    {
        if(i < command_count-1) // if theres another command after this one, pipe is needed to connect them otherwise it already exists from the previous one
        {
            pipe(pipe_file_descriptors);
            output_file_descriptor = pipe_file_descriptors[1];
        }
        else
        {
            output_file_descriptor = 1;
        }
        int pid = fork();
        if(pid != 0)
        {
            pids.push_back(pid); // pid is added to list of child processes
            if(output_file_descriptor != 1)
            {
                close(output_file_descriptor);
            }
            if(input_file_descriptor != 0)
            {
                close(input_file_descriptor);
            }
            input_file_descriptor = pipe_file_descriptors[0];
        }
        else
        {
            char * command[MAX_ARG];
            int pipe_count = 0, arg_count = 0;
            for(int j = 0; j < token_amount; j++) // this for loop uses a counter for the pipes as well as the i iterator
            {                                     // to figure out which command of the pipeline it should execute
                if(strcmp(commands[j],"|") == 0)  // and get all of its arguments
                {
                    pipe_count++;
                    continue;
                }
                if(pipe_count == i)
                {
                    command[arg_count++] = commands[j];
                }
                else if(pipe_count > i)
                {
                    break;
                }
            }
            command[arg_count] = NULL;
            if(input_file_descriptor != 0)  //pipes used to connect STDIN, STDOUT if needed
            {
                dup2(input_file_descriptor,0);
                close(input_file_descriptor);
            }
            if(output_file_descriptor != 1)
            {
                dup2(output_file_descriptor,1);
                close(output_file_descriptor);
                close(pipe_file_descriptors[0]);
            }
            io_redirection(command,arg_count); //command passed into io redirection function where it will be executed in the end
        }
    }
    if(is_foreground_pipeline)//if the pipeline isnt being ran in the background need to call waitpid for all the child processes
    {
        int status;
        for(auto i = pids.begin(); i != pids.end(); i++)
        {
            waitpid(*i, &status, 0);
        }
    }
}

int main()
{
    //signal handler for SIGTSTP and SIGINT
    struct sigaction sa1, sa2;
    sa1.sa_flags = SA_RESTART;
    sa1.sa_handler = &handle_signal;
    sa2.sa_flags = SA_RESTART;
    sa2.sa_handler = &handle_signal;
    sigaction(SIGTSTP, &sa1, NULL);
    sigaction(SIGINT, &sa2, NULL);

    char input[MAX_INPUT];
    while(1)
    {
        cout << "in-mysh-now:>";
        cin.getline(input, MAX_INPUT);

        //this section tokenizes the input, puts spaces in between characters >, <, |, &, ;, ", 
        //example: turns cat file1|sort>file2 into cat file1 | sort > file2
        char tokenized_input[MAX_INPUT];
        int i = 0, j = 0;
        while(input[i] != '\0')
        {
            if(input[i] == '>' || input[i] == '<' || input[i] == '|' || input[i] == '&' || input[i] == ';' || input[i] == '"')
            {
                tokenized_input[j++] = ' ';
                tokenized_input[j++] = input[i++];
                tokenized_input[j++] = ' ';
            }
            else
            {
                tokenized_input[j++] = input[i++];
            }
        }
        tokenized_input[j] = '\0';


        char * token = strtok(tokenized_input, " \n");
        if(strcmp(token,"exit") == 0)//exit command
        {
            exit(0);
        }
        while( token != NULL){
            int token_amount = 0;
            char * tokens[MAX_TOKEN];

            //this segment takes arguments that are in between double quotes and turns them into one argument
            while( token != NULL )
            {
                if(strcmp(token,"\"") == 0)
                {
                    char temp_token[] = "";
                    token = strtok(NULL," \n");
                    while(strcmp(token,"\"") != 0)
                    {
                        strcat(temp_token, token);
                        strcat(temp_token, " ");
                        token = strtok(NULL," \n");
                    }
                    tokens[token_amount++] = temp_token;
                    token = strtok(NULL," \n");
                }
                else if(strcmp(token,";") == 0)//break loop if ; is found to execute
                {
                    token = strtok(NULL," \n");
                    break;
                }

                //this part handles wildcard characters

                else if(strchr(token,'*') != NULL || strchr(token,'?') != NULL)
                {
                    char **found;
                    glob_t gstruct;
                    int r;
                    r = glob(token, GLOB_ERR , NULL, &gstruct);
                    if( r!=0 )
                    {
                        if( r==GLOB_NOMATCH )
                            fprintf(stderr,"No matches\n");
                        else
                            fprintf(stderr,"Glob error\n");
                        exit(1);
                    }
                    found = gstruct.gl_pathv;
                    while(*found)
                    {
                        tokens[token_amount++] = *found;
                        found++;
                    }
                    token = strtok(NULL, " \n");
                }
                else
                {
                    tokens[token_amount++] = token;
                    token = strtok(NULL, " \n");
                }
            }
            pipe_commands(tokens, token_amount);
        }
        int status, pid;
        while((pid = waitpid(-1, &status, WNOHANG)) > 0)//this loop receives the exit codes of background processes if they are finished
        {                                       //without having to hang if they havent finished execution yet
            cout << "Background process "<< pid << " ended" << endl;
        }
    }
}