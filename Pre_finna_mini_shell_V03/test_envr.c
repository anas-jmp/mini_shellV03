#include "mini_shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Access the environment
extern char **environ;
//--------------------------------//
void handle_unset(const char *key) 
{
    t_env *current = env_list;
    t_env *previous = NULL;

    while (current)
     {
        if (strcmp(current->key, key) == 0) 
        {
            if (previous == NULL)
            {
                env_list = current->next;   
            } 
            else
            {
                previous->next = current->next;
            }
            free(current->key);
            free(current->value);
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void handle_env()
{
    t_env *current = env_list;
    t_env *previous = NULL;

    char **env = environ;
      
    while(*env)
    {
            printf("%s\n",*env);
            env++;

    }
    printf("\ngood job again\n");
    while(current)
    {
        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>ENV_Stored variable: %s=%s\n", current->key, current->value); 
        previous = current;
        current = current->next;
    } // debugging output
}
