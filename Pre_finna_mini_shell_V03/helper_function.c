#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mini_shell.h"


t_env *env_list = NULL;  // Initialize the custom environment list


// Split "var=value" into key and value
void split_key_value(char *input, char **key, char **value)
{
    char *equal_sign = strchr(input, '=');
    if (equal_sign)
    {
        int key_len = equal_sign - input;
        int val_len = strlen(equal_sign + 1);

        *key = malloc(key_len + 1);
        *value = malloc(val_len + 1);

        if (*key && *value)
        {
            strncpy(*key, input, key_len);  
            (*key)[key_len] = '\0';         // Null-terminate key
            strcpy(*value, equal_sign + 1); // cpy value part after '='
        }
        else
        {
            free(*key);  
            free(*value);
            *key = NULL;
            *value = NULL;
        }
    }
    else
    {
        *key = NULL;
        *value = NULL;
    }
}

char *expand_variable(char *var_name, char **env)
{
    // first, check custom shell environment (env_list)
    t_env *current = env_list;
    while (current)
    {
        if (strcmp(current->key, var_name) == 0)
        {
            //printf("Match found in shell env for %s: %s\n", var_name, current->value);  // Debug print
            return strdup(current->value);  // return the shell variable's value
        }
        current = current->next;
    }

    // If not found, check system environment (env[])
    for (int i = 0; env[i]; i++)
    {
        if (strncmp(env[i], var_name, strlen(var_name)) == 0 && env[i][strlen(var_name)] == '=')
        {
           // printf("match found in system env for %s: %s\n", var_name, env[i] + strlen(var_name) + 1);  // Debug print
            return strdup(env[i] + strlen(var_name) + 1);  // return system environment variables value
        }
    }
    return NULL;
}
