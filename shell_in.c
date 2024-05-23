#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/wait.h>

#define BUFSIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"


/**
 *  Function declarations for built-in shell commands
*/
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_cls(char **args);

/**
 *  List of buil-in commands, followed by their corresponding functions.
*/
char *builtin_str[] =  {
                         "cd",
                         "help",
                         "exit",
                         "cls",
                       };

int (*builtin_funcptr[]) (char **) =  {
                                        &shell_cd,
                                        &shell_help,
                                        &shell_exit,
                                        &shell_cls,
                                      };

int shell_num_builtin();
void init_loop();
char *read_line(void);
char **split_line(char *line);
int launch(char **args);
int shell_execute(char **args);

int main(int argc, char *argv[])
{
    init_loop();

    return 0;
}

void init_loop(void)
{
    char *line;
    char **args;
    int status;

    char cwd[PATH_MAX];
    do
    {
        getcwd(cwd, sizeof(cwd));
        printf("%s>> ", cwd);
        printf("\033[0m");
        line = read_line();
        args = split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while (status);

}

char *read_line(void)
{
    int bufsize = BUFSIZE;
    int position = 0;
    int c;
    char *buffer = malloc(sizeof(*buffer) * bufsize);
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    while (1)
    {
        // Read a character
        c = getchar();
        // If user inputs EOF or newline then we return our string after null terminating it.
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;


        // If we need to reallocate
        if (position >= BUFSIZE)
        {
            bufsize += BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (buffer == NULL)
            {
                fprintf(stderr, "Reallocation failed\n");
                exit(2);
            }
        }
    }
}

char **split_line(char *line)
{
    int bufsize = TOK_BUFSIZE;
    int position = 0;
    char *token;
    char **tokens = malloc(bufsize * sizeof(char*));
    if (tokens == NULL)
    {
        fprintf(stderr, "Allocation failed\n");
        exit(3);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (tokens == NULL)
            {
                fprintf(stderr, "tokens realloc failed\n");
                exit(4);
            }
        }

        token = strtok(NULL, TOK_DELIM);
    }

    // check for "&"
    if ((position > 0) && (!strcmp("&", tokens[position - 1])))
    {
        tokens[position - 1] = NULL;
        tokens[position] = "&";
    }
    else
    {
        tokens[position] = NULL;
    }
    return tokens;
}

int shell_execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        return 1;
    }

    // loops through every function in the array
    for (i = 0; i < shell_num_builtin(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_funcptr[i])(args);
        }
    }
    return launch(args);
}

int shell_num_builtin()
{
    return sizeof(builtin_str) / sizeof(char *);
}

/**
 *  Builtin functions implementations
*/

int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, ">>: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror(">>");
        }
    }
    return 1;
}

int shell_help(char **args)
{
    int i;
    printf("Credits to Stephen Brennan's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < shell_num_builtin(); i++)
    {
        printf("    %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int shell_exit(char **args)
{
    return 0;
}

int shell_cls(char **args)
{
    printf("\033c");
    return 1;
}
int launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();

    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("Shell");
        }
        exit(1);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("Shell");
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}
