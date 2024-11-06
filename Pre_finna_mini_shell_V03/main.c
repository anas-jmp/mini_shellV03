#include "mini_shell.h"
#include <sys/wait.h>
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

int ft_lstsize(t_command *lst)
{
    int lst_size = 0;
    while (lst) {
        lst_size++;
        lst = lst->next;
    }
    return lst_size;
}
 
pid_t handle_child_process(t_command *prompt, char *cmd_path, char **argv, char **env, int **pipefd, int j, int lst_size) 
{
    pid_t pid;

    int fd;
    int len,len2,len3;
    char *var,*var3,*var2;
    char *final;
    int i; 
    if (!strcmp(prompt->command, "cd")) 
    {
        ft_cd((prompt->args)[1]);
        return 0;
    }    
  if (!strcmp(prompt->command, "export"))
     {
        printf("\n@@ Handling export command\n");
        // printf("\nthe value of , prompt->args[%d] : %s\n",1,prompt->args[1]);
        // printf("\nthe value of , prompt->args[%d] : %s\n",2,prompt->args[2]);
        // printf("\nthe value of , prompt->args[%d] : %s\n",3,prompt->args[3]);
        // printf("\nthe value of , prompt->args[%d] : %s\n",4,prompt->args[4]);
        for ( i = 1; prompt->args[i]; i++) 
        {
              printf("\nthe value of , prompt->args[%d] : %s\n",i,prompt->args[i]);
        }
        printf("\n%d i -->\n",i);
         if (i==2)
        {      
             printf("\nfirst part%d\n ",i);
            for ( int x = 1; prompt->args[x]; x++) 
            {
                printf("\nthe value of , prompt->args[%d] : %s\n",x,prompt->args[x]);
                handle_export_command(prompt->args[x],env);  // Pass the entire argument array and env
            }
        }
        printf("\nxxx_nmb of command %d\n ",i);
        if(i==3)
        {
         printf("\nmiddle part%d\n ",i);
         printf("\nnmb of command%d\n ",i);
         len = strlen(prompt->args[1]);
          len2 = strlen(prompt->args[2]);
         printf("the len is : %d",len );
         var=malloc(len);
          var2=malloc(len2);
         strcpy(var,prompt->args[1]);
         strcpy(var2,prompt->args[2]);
         final=malloc(len+len2+1);
         strcpy(final,var);
         strcat(final,var2);  
         printf("\n%s \n",final);
         handle_export_command(final,env);  // Pass the entire argument array and env
     }

     if(i==4)
        {
         printf("\nlast part%d\n ",i);
         printf("\nnmb of command%d\n ",i);
         len  = strlen(prompt->args[1]);
         len2 = strlen(prompt->args[2]);
         len3 = strlen(prompt->args[3]);

         printf("the len is : %d",len );
         var  = malloc(len);
         var2 = malloc(len2);
         var3 = malloc(len3);
         strcpy(var,prompt->args[1]);
         strcpy(var2,prompt->args[2]);
         strcpy(var3,prompt->args[3]);
         final=malloc(len+len3+len2+1);
         strcpy(final,var);
         strcat(final,var2);
         strcat(final,var3);
         final[len+len3+len2+1]='\0';
         handle_export_command(final,env);  // Pass the entire argument array and env
     }  
    return 0;
    
    }
    
    if (!strcmp(prompt->command, "unset"))
    {
        printf("\n unset command called\n");
    }
    // Fork a new process
    pid = fork();
    if (pid < 0) 
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
     {  // Child process
        // Handle pipe redirections based on the position in the pipeline
        if (j > 0) {
            dup2(pipefd[j - 1][0], STDIN_FILENO); // Input from the previous pipe
        }
        if (j < lst_size - 1) {
            dup2(pipefd[j][1], STDOUT_FILENO); // Output to the current pipe
        }

        // Close all pipe file descriptors in child
        for (int i = 0; i < lst_size - 1; i++)
        {
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }

        // Handle input redirections
        for (int i = 0; prompt->input_redirects && prompt->input_redirects[i]; i++)
        {
            fd = open(prompt->input_redirects[i], O_RDONLY);
            if (fd < 0) {
                perror(prompt->input_redirects[i]);
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Handle output redirections
        for (int i = 0; prompt->output_redirects && prompt->output_redirects[i]; i++)
         {
            if (prompt->append_modes[i])
            {
                fd = open(prompt->output_redirects[i], O_WRONLY | O_CREAT | O_APPEND, 0644);
            } 
            else 
            {
                fd = open(prompt->output_redirects[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd < 0)
            {
                perror(prompt->output_redirects[i]);
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
          execute_command(cmd_path, argv, env);
          exit(EXIT_FAILURE);  // Exit if execve fails

        }
    return pid;
}



pid_t middle_child_processes(t_command *prompt, char *cmd_path, char **argv, int **pipefd, char **env, int j)
 {
    pid_t pid;
    int fd;
    
    if (!strcmp(prompt->command, "cd"))
    {
        ft_cd((prompt->args)[1]);
        return 0;
    }
    // Fork a new process
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {  // Child process
        dup2(pipefd[j - 1][0], STDIN_FILENO);
        dup2(pipefd[j][1], STDOUT_FILENO);
        close(pipefd[j - 1][1]);
        close(pipefd[j][0]);
        // printf("Middle Child PID %d: Input from pipe %d and output to pipe %d\n", getpid(), j - 1, j);

        // Handle input redirections
        for (int i = 0; prompt->input_redirects && prompt->input_redirects[i]; i++)
        {
            fd = open(prompt->input_redirects[i], O_RDONLY);
            if (fd < 0) {
                perror(prompt->input_redirects[i]);
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            // printf("Middle Child PID %d: Redirecting input from %s\n", getpid(), prompt->input_redirects[i]);
        }

        // Handle output redirections
        for (int i = 0; prompt->output_redirects && prompt->output_redirects[i]; i++) {
            if (prompt->append_modes[i]) {
                fd = open(prompt->output_redirects[i], O_WRONLY | O_CREAT | O_APPEND, 0644);
                // printf("Middle Child PID %d: Appending output to %s\n", getpid(), prompt->output_redirects[i]);
            } else {
                fd = open(prompt->output_redirects[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                // printf("Middle Child PID %d: Redirecting output to %s\n", getpid(), prompt->output_redirects[i]);
            }
            if (fd < 0) {
                perror(prompt->output_redirects[i]);
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        close(pipefd[j - 1][0]);
        close(pipefd[j][1]);
        execute_command(cmd_path, argv, env);
    }
    return pid;
}

pid_t ending_child_process(t_command *prompt, char *cmd_path, char **argv, int **pipefd, char **env, int j) 
{
    pid_t pid;
    int fd;

    if (!strcmp(prompt->command, "cd"))
    {
        ft_cd((prompt->args)[1]);
        return 0;
    }
    // Fork a new process
    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
     {  // Child process
        dup2(pipefd[j - 1][0], STDIN_FILENO);
        close(pipefd[j - 1][1]);
        // printf("Ending Child PID %d: Input from pipe %d\n", getpid(), j - 1);

        // Handle input redirections
        for (int i = 0; prompt->input_redirects && prompt->input_redirects[i]; i++) 
        {
            fd = open(prompt->input_redirects[i], O_RDONLY);
            if (fd < 0)
            {
                perror(prompt->input_redirects[i]);
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            // printf("Ending Child PID %d: Redirecting input from %s\n", getpid(), prompt->input_redirects[i]);
        }

        // Handle output redirections
        for (int i = 0; prompt->output_redirects && prompt->output_redirects[i]; i++) 
        {
            if (prompt->append_modes[i])
             {
                fd = open(prompt->output_redirects[i], O_WRONLY | O_CREAT | O_APPEND, 0644);
                // printf("Ending Child PID %d: Appending output to %s\n", getpid(), prompt->output_redirects[i]);
            } else {
                fd = open(prompt->output_redirects[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                // printf("Ending Child PID %d: Redirecting output to %s\n", getpid(), prompt->output_redirects[i]);
            }
            if (fd < 0) {
                perror(prompt->output_redirects[i]);
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execute_command(cmd_path, argv, env);
    }
    return pid;
}


int pipex(t_command *prompt, char **env)
 {
    int lst_size = ft_lstsize(prompt);
    int **pipefd;
    int status;
    char *cmd_path;
    pid_t pid;
    int j = 0;

    // Allocate pipes
    pipefd = malloc(sizeof(int *) * (lst_size - 1));
    if (!pipefd) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < lst_size - 1; i++) 
    {
        pipefd[i] = malloc(sizeof(int) * 2);
        if (pipe(pipefd[i]) < 0) 
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Iterate over each command in the pipeline
    while (prompt) 
    {
        cmd_path = is_command(prompt->command, env);
        if (!cmd_path)
        {
            cmd_path = prompt->command;
        }

        pid = handle_child_process(prompt, cmd_path, prompt->args, env, pipefd, j, lst_size);

        // Move to the next command in the pipeline
        prompt = prompt->next;
        j++;
    }

    // Close all pipes in parent process
    for (int i = 0; i < lst_size - 1; i++)
    {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }

    // Wait for all child processes
    for (int i = 0; i < lst_size; i++)
    {
        wait(&status);
    }

    // Free pipe file descriptors
    for (int i = 0; i < lst_size - 1; i++)
    {
        free(pipefd[i]);
    }
    free(pipefd);

    return 0;
}

 
void    is_exit_with_signal(int *status)
{
    if (WIFEXITED(*status))
        *status = WEXITSTATUS(*status);
    else if (WIFSIGNALED(*status))
        *status = WTERMSIG(*status) + 128;
} 

int main(int argc , char **argv , char **env)
{
   
    (void)argc;
    (void)argv;
     
     display_ascii_art(); 
     shell_loop(env);

    return 0;
}

void shell_loop(char **env) 
{
    char *line;
    token *tokens;
    t_command *parsed_command;
    int flag=0;

    while (1) 
    {
        flag = 0;
        printf("\n+-+\n");
        line = readline("\x1B[31mMini_shell(*-*)>\x1B[37m");  // Prompt for input
        //check_unmatched_quotes(*line);
        if (!line)
        {
            printf("\nexit\n");
            exit(0);
        }
          if (line[0] == '\0')
        {
            free(line);
            continue;
        }
        else
        {
            // Tokenize the input line for general commands
            tokens = tokenize_input(line, env);  // Updated function name
        
              if(check_unmatched_quotes(line)==1 || check_unmatched_redirection(line)==1 || check_unmatched_pipes(line)==1)
                {
                    printf("\nGOOD...\n");
                     flag=1;
                }
                if (strcmp(line, "clear") == 0)
                    {
                        system("clear");
                        display_ascii_art();  
                        free(line);
                        continue;
                    }
            if (tokens && flag==0)
            {
                // Parse the tokens into a command structure
                parsed_command = parse_command(tokens, env);

                // Execute the parsed commands
                pipex(parsed_command, env);
                // Free the parsed command and token list
                free_token_list(tokens);
                free_command(parsed_command);
            }
        }
        add_history(line);  // Store the input in history
        free(line);  // Free the line buffer
    }
}
 