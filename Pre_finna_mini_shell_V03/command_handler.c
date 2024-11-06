#include <stdio.h>
#include <string.h>
#include "mini_shell.h"

    // ORGN
void store_env_variable(char *key, char *value)
{
    t_env *current = env_list;
      //  counte_env=strlen();
    //   check if the variable already exists and update if found
    while (current)
    {
        if (strcmp(current->key, key) == 0)
        {
            free(current->value);          // free old value
            current->value = strdup(value); //update with new value
            return;
        }
        current = current->next;
    }

     t_env *new_env = malloc(sizeof(t_env));
    if (!new_env)
        return;

    new_env->key = strdup(key);
    new_env->value = strdup(value);
    new_env->next = env_list;
    env_list = new_env;  
     // handle_env(key,value); 
     
      printf("1)Stored variable: %s=%s\n", new_env->key, new_env->value);  
      printf("2)Stored variable: %s=%s\n", key, value);  
}



/////  orgin
// handle the "expand" command to store variables like "var=value"
void handle_export_command(char *input, char **env)
 {
    char *key = NULL;
    char *value = NULL;

    split_key_value(input, &key, &value);  // Split "var=value" into key and value

    if (key && value) {
        printf("\n?after split_key_value\n");
        char *expanded_value = expand_variable(value + 1, env);  // Expand variable (skip the $ sign)
        if (expanded_value) {
            store_env_variable(key, expanded_value);  // Store the variable in env_list
            free(expanded_value);  // Free the expanded value after storing
        } else {
            store_env_variable(key, value);  // If no expansion, store as is
        }
        free(key);
        free(value);
    }
    else
    {
        printf("Invalid format: %s\n", input);
        if (key) free(key);  // Free allocated key if it was allocated
        if (value) free(value);  // Free allocated value if it was allocated
    }
}

    
void handle_echo_command(char *input, char **env)
{
     int in_single_quote = 0;
     int in_double_quote = 0;
     char buffer[1024] = {0};  // Temporary buffer for building output
     int buf_idx = 0;
     int found_unrecognized_var = 0;
     int suppress_newline = 0;  // Flag to suppress the trailing newline

    // Process options
    while (*input == '-' && *(input + 1) == 'n')
    {
        suppress_newline = 1;  // Set flag to suppress newline
        input += 2;  // Move past '-n'
        // Skip any whitespace after the option
        while (isspace(*input)) input++;
    }

    while (*input) {
        // Handle single quote (toggle single quote mode)
        if (*input == '\'' && !in_double_quote)
        {
            in_single_quote = !in_single_quote;  // Toggle single quote mode
            input++;  // Skip the single quote
            continue;
        }

        // Handle double quote (toggle double quote mode)
        if (*input == '"' && !in_single_quote)
        {
            in_double_quote = !in_double_quote;  // Toggle double quote mode
            input++;  // Skip the double quote
            continue;
        }

        // Handle escape sequences
        if (*input == '\\' && (in_double_quote || !in_single_quote))
        {
            input++;  // Move to the character after the backslash
            if (*input == '"' || *input == '\'' || *input == '\\')
            {
                buffer[buf_idx++] = *input;  // Add the escaped character
            }
            else
            {
                buffer[buf_idx++] = '\\';  // Just add the backslash if it's not followed by a special character
            }
            input++;  // Move to the next character
            continue;
        }

        // If inside single quotes, treat as literal text (no expansion)
        if (in_single_quote)
        {
            buffer[buf_idx++] = *input;  // Copy literal characters inside single quotes
        }
        // If inside double quotes or unquoted, handle variable expansion
        else if (*input == '$' && !in_single_quote)
        {
            buffer[buf_idx] = '\0';  // End the current buffer (before $)
          //  printf("w%s", buffer);  // Print the buffer content before variable
            buf_idx = 0;

            // Parse variable name
            input++;  // Skip the '$'
            char var_name[256] = {0};  // Buffer for variable name
            int var_idx = 0;

            while (isalnum(*input) || *input == '_')  // Capture valid variable characters
            {
                var_name[var_idx++] = *input++;
            }
            var_name[var_idx] = '\0';  // Null-terminate variable name

            // Expand the variable
            char *expanded_value = expand_variable(var_name, env);
            if (expanded_value)
            {
                printf(">%s", expanded_value);  // Print expanded value
                free(expanded_value);  // Free allocated memory
            }
            else
            {
                found_unrecognized_var = 1; 
                // Uncomment the next line if you want to print unrecognized variables
                 //printf("Warning: Unrecognized variable: %s\n", var_name);
            }
            continue;  // Continue to next character
        }
        else
        {
            // Add regular characters to the buffer
            buffer[buf_idx++] = *input;
        }

        input++;  // Move to the next character
    }

    // If there's any remaining buffer, print it
    if (buf_idx > 0)
    {
        buffer[buf_idx] = '\0';
        printf("x%s", buffer);  // Print the remaining buffer
    }
    if (found_unrecognized_var)
    {
        printf("Warning: Unrecognized variable(s) printed.\n");
    }
    printf("\nhint\n");
    // print a new line unless suppressed
    if (!suppress_newline)
    {
        printf("\n");
    }
}
