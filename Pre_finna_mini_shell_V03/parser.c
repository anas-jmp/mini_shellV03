#include "mini_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_REDIRECT_SIZE 4  // Initial size for redirection arrays
#define INITIAL_ARGS_SIZE 16

/**/
char* handle_echo_command2(char *input, char **env)
{
    //printf("\nhandle_echo_command2\n");
    int in_single_quote = 0;
    int in_double_quote = 0;
    char buffer[1024] = {0};  // temporary buffer for building output
    int buf_idx = 0;
    int found_unrecognized_var = 0;
    int suppress_newline = 0;  // flag to suppress the trailing newline

    // final output buffer
    char *output = malloc(2048);  
    if (!output) return NULL;
    output[0] = '\0';  // initialize as an empty string

    // process options
    while (*input == '-' && *(input + 1) == 'n') {
        suppress_newline = 1;  // set flag to suppress newline
        input += 2;  // move past '-n'
        // skip any whitespace after the option
        while (isspace(*input)) input++;
    }

    while (*input) {
        // handle single quote (toggle single quote mode)
        if (*input == '\'' && !in_double_quote) {
            in_single_quote = !in_single_quote;
            input++;
            continue;
        }

        // handle double quote (toggle double quote mode)
        if (*input == '"' && !in_single_quote) {
            in_double_quote = !in_double_quote;
            input++;
            continue;
        }

        // handle escape sequences
        if (*input == '\\' && (in_double_quote || !in_single_quote))
        {
            input++;
            if (*input == '"' || *input == '\'' || *input == '\\') {
                buffer[buf_idx++] = *input;
            } else {
                buffer[buf_idx++] = '\\';
            }
            input++;
            continue;
        }

        // if inside single quotes, treat as literal text
        if (in_single_quote)
        {
            buffer[buf_idx++] = *input;
        } 
        // handle variable expansion
        else if (*input == '$' && !in_single_quote)
        {
            buffer[buf_idx] = '\0';
            strcat(output, buffer);  // append buffer to output
            buf_idx = 0;

            input++;
            char var_name[256] = {0};
            int var_idx = 0;
            while (isalnum(*input) || *input == '_')
            {
                var_name[var_idx++] = *input++;
            }
            var_name[var_idx] = '\0';

            char *expanded_value = expand_variable(var_name, env);
            if (expanded_value) {
                strcat(output, expanded_value);  // append expanded value to output
                free(expanded_value);
            } 
            else 
            {
                found_unrecognized_var = 1;
            }
            continue;
        } 
        else 
        {
            buffer[buf_idx++] = *input;
        }

        input++;
    }

    // Append any remaining buffer content to output
    if (buf_idx > 0)
    {
        buffer[buf_idx] = '\0';
        strcat(output, buffer);
    }

    if (found_unrecognized_var)
    {
        strcat(output, "Warning: Unrecognized variable(s) printed.\n");
    }

    if (!suppress_newline) {
        strcat(output, "\n");
    }
   return output;
}
/** */
char **resize_array(char **old_array, int old_size, int new_size)
{
    char **new_array = malloc(new_size * sizeof(char *));
    if (!new_array) {
        return NULL;
    }

    // Copy old array content to new array
    for (int i = 0; i < old_size; i++) {
        new_array[i] = old_array[i];
    }

    // Free old array
    free(old_array);

    return new_array;
}

// Helper function to resize an int array (for append_modes)
int *resize_int_array(int *old_array, int old_size, int new_size)
{
    int *new_array = malloc(new_size * sizeof(int));
    if (!new_array) {
        return NULL;
    }

    // Copy old array content to new array
    for (int i = 0; i < old_size; i++) {
        new_array[i] = old_array[i];
    }

    // Free old array
    free(old_array);

    return new_array;
}

void free_command(t_command *cmd)
{
    if (cmd) {
        free(cmd->command);
        if (cmd->args) {
            for (int i = 0; cmd->args[i]; i++)
            {
                free(cmd->args[i]);
            }
            free(cmd->args);
        }
        if (cmd->input_redirects) {
            for (int i = 0; cmd->input_redirects[i]; i++)
            {
                free(cmd->input_redirects[i]);
            }
            free(cmd->input_redirects);
        }
        if (cmd->output_redirects)
        {
            for (int i = 0; cmd->output_redirects[i]; i++)
            { 
                free(cmd->output_redirects[i]);
            }
            free(cmd->output_redirects);
        }
        free(cmd->append_modes);
        free(cmd);
    }
}

t_command *parse_command(token *tokens, char **env) 
{
    if (!tokens) 
        return NULL;

    t_command *head = NULL;
    t_command *current = NULL;

    while (tokens) {
        // allocate and initialize a new t_command structure
        t_command *cmd = malloc(sizeof(t_command));
        if (!cmd) 
            return NULL;

        cmd->command = NULL;
        cmd->args = malloc(INITIAL_ARGS_SIZE * sizeof(char *));
        cmd->input_redirects = malloc(INITIAL_REDIRECT_SIZE * sizeof(char *));
        cmd->output_redirects = malloc(INITIAL_REDIRECT_SIZE * sizeof(char *));
        cmd->append_modes = malloc(INITIAL_REDIRECT_SIZE * sizeof(int));
        cmd->next = NULL;

        if (!cmd->args || !cmd->input_redirects || !cmd->output_redirects || !cmd->append_modes) {
            free_command(cmd);
            return NULL;
        }

        int arg_count = 0, max_args = INITIAL_ARGS_SIZE;
        int input_count = 0, max_inputs = INITIAL_REDIRECT_SIZE;
        int output_count = 0, max_outputs = INITIAL_REDIRECT_SIZE;

        // parse tokens for the current command until a pipe or end of tokens
        while (tokens && tokens->token_type != PIPES) {
            // Handle the command name
            if (!cmd->command && (tokens->token_type == WORD || 
                tokens->token_type == DOUBLE_QUOTED_STRING || tokens->token_type == SINGLE_QUOTED_STRING)) {
                cmd->command = strdup(tokens->value);
                cmd->args[arg_count++] = strdup(cmd->command);
                tokens = tokens->next;
                continue;
            }

            // handle arguments
            if (tokens->token_type == WORD || 
                tokens->token_type == DOUBLE_QUOTED_STRING || tokens->token_type == SINGLE_QUOTED_STRING)
                 {
                char *arg_value = tokens->value;

                // check for variable expansion
                if (arg_value[0] == '$') {
                    char *var_name = arg_value + 1;  // Skip the '$'
                    char *expanded_value = expand_variable(var_name, env);
                    if (expanded_value) {
                        arg_value = expanded_value;  // Use the expanded value
                    }
                }

                // resize args array if necessary
                if (arg_count >= max_args - 1) {
                    max_args *= 2;
                    cmd->args = resize_array(cmd->args, arg_count, max_args);
                    if (!cmd->args) {
                        free_command(cmd);
                        return NULL;
                    }
                }
                cmd->args[arg_count++] = strdup(arg_value);
            }

            // handle input redirection
            else if (tokens->token_type == INPUT_REDIRECTION)
            {
                tokens = tokens->next;  // Move to the file name
                if (tokens && (tokens->token_type == WORD || 
                    tokens->token_type == DOUBLE_QUOTED_STRING || tokens->token_type == SINGLE_QUOTED_STRING)) {
                    // Resize input_redirects array if necessary
                    if (input_count >= max_inputs - 1) {
                        max_inputs *= 2;
                        cmd->input_redirects = resize_array(cmd->input_redirects, input_count, max_inputs);
                        if (!cmd->input_redirects) {
                            free_command(cmd);
                            return NULL;
                        }
                    }
                    cmd->input_redirects[input_count++] = strdup(tokens->value);
                }
                 else 
                 {
                    printf("\nError: No file specified for input redirection\n");
                    free_command(cmd);
                    return NULL;
                }
            }

            // Handle output redirection
            else if (tokens->token_type == OUTPUT_REDIRECTION || tokens->token_type == OUTPUT_REDIRECTION_APPEND_MODE)
             {
                int append = (tokens->token_type == OUTPUT_REDIRECTION_APPEND_MODE);
                tokens = tokens->next;  // Move to the file name
                if (tokens && (tokens->token_type == WORD || 
                    tokens->token_type == DOUBLE_QUOTED_STRING || tokens->token_type == SINGLE_QUOTED_STRING)) {
                    // Resize output_redirects and append_modes arrays if necessary
                    if (output_count >= max_outputs - 1) {
                        max_outputs *= 2;
                        cmd->output_redirects = resize_array(cmd->output_redirects, output_count, max_outputs);
                        cmd->append_modes = resize_int_array(cmd->append_modes, output_count, max_outputs);
                        if (!cmd->output_redirects || !cmd->append_modes) {
                            free_command(cmd);
                            return NULL;
                        }
                    }
                    cmd->output_redirects[output_count] = strdup(tokens->value);
                    cmd->append_modes[output_count] = append;
                    output_count++;
                } else {
                    free_command(cmd);
                    return NULL;
                }
            }

            tokens = tokens->next;
        }

        cmd->args[arg_count] = NULL;  // Null-terminate the args array
        cmd->input_redirects[input_count] = NULL;  // Null-terminate the input redirects array
        cmd->output_redirects[output_count] = NULL;  // Null-terminate the output redirects array

        // Add the command to the linked list
        if (!head) 
        {
            head = cmd;
        } 
        else 
        {
            current->next = cmd;
        }
        current = cmd;

        // Move past the pipe if there is one
        if (tokens && tokens->token_type == PIPES)
         {
            tokens = tokens->next;
        }
    }

    return head;
}


token *tokenize_input(const char *input, char **env)
{
    token *head = NULL;
    token *current = NULL;
    const char *str = input;

    while (*str)
    {
        // Skip leading whitespace
        while (isspace(*str))
            str++;

        if (*str == '\0')
            break;

        char *token_value = NULL;
        token_type type;  // Declare a variable to hold the token type

        // Get the next token (handles quoted strings)
         token_value = next_token(&str, env, &type);  // Pass the type variable to next_token

        if (token_value == NULL)
            continue;  // Skip invalid or empty tokens

        // Create new token node
        token *new_token = malloc(sizeof(token));
        new_token->value = token_value;
        new_token->token_type = type;
        new_token->next = NULL;
       // printf("Tokenized value: %s, Type: %d\n", new_token->value, new_token->token_type);

        // Append to the token list
        if (!head)
            head = new_token;
        else
            current->next = new_token;
        current = new_token;
    }

    return head;
}

   // ORGIN
char *next_token(const char **str, char **env, token_type *type)
{
    // Skip leading whitespace
    while (isspace(**str))
        (*str)++;

    if (**str == '\0')
        return NULL;
    const char *start = *str;
// Handle 'echo' command

if (strncmp(start, "echo", 4) == 0 && (isspace(start[4]) || start[4] == '\0'))
{
    *str += 4;  // Move past 'echo'
       // printf("\n+++ok\n");
    // Create a mutable copy of the remaining input
    char *remaining_input = strdup(*str);
    if (remaining_input == NULL)
    {
        // Handle memory allocation failure
        perror("strdup");
        exit(EXIT_FAILURE);
    }
    // Call handle_echo_command with the mutable copy
    handle_echo_command(remaining_input, env);  // Process the remaining input for echo
    free(remaining_input);  // Free the allocated memory for remaining_input
    //return NULL;  // Return NULL as we don't need to process further tokens
}
  
if (strncmp(start, "unset", 5) == 0 && (isspace(start[5]) || start[5] == '\0'))
{
    *str += 5;  // Move past 'unset'
    
    // Skip any leading whitespace
    while (isspace(**str))
    {
        (*str)++;
    }
    
    // Extract the variable name
    const char *variable_name = *str;//strtok(*str, " \t\n");
    if (variable_name)
    {
        handle_unset(variable_name);
    }
}

if (strncmp(start, "env", 3) == 0 && (isspace(start[3]) || start[3] == '\0'))
{
    *str += 3;  // Move past 'echo'
     printf("\nenvirment command found\n");
     handle_env();

}
    // // Handle quoted strings
    if (**str == '"' || **str == '\'')
    {
        char quote = **str;  // Get the quote character (' or ")
        (*str)++;  // Skip past the opening quote

        // Extract the quoted string
        const char *quoted_start = *str;
        while (**str && **str != quote)
            (*str)++;
        size_t len = *str - quoted_start;
        char *token_value = malloc(len + 1);
         strncpy(token_value, quoted_start, len);
        token_value[len] = '\0';

        if (**str == quote)
            (*str)++;  // Skip past the closing quote

        if (quote == '\'')
        {
            // Single-quoted string: No expansion
            *type = SINGLE_QUOTED_STRING;
            return token_value;
        }
 
         else
        {
            // printf("\nelse echo\n");
            // Handle double-quoted strings
            *type = DOUBLE_QUOTED_STRING;
            char *expanded_token = handle_echo_command2(token_value, env);
            free(token_value);
            return expanded_token;
        }
    }

    // handle redirection and pipes
    if (**str == '>' || **str == '<' || **str == '|')
    {
        char symbol = **str;
        (*str)++;  // Move past the symbol

        if (symbol == '>' && **str == '>')
        {
            (*str)++;  // Handle '>>'
            *type = OUTPUT_REDIRECTION_APPEND_MODE;
            return strdup(">>");
        }
        else if (symbol == '<' && **str == '<')
        {
            (*str)++;  // Handle '<<'
            *type = HERE_DOC;
            return strdup("<<");
        }
        else
        {
            char temp[2] = {symbol, '\0'};
            if (symbol == '>')
                *type = OUTPUT_REDIRECTION;
            else if (symbol == '<')
                *type = INPUT_REDIRECTION;
            else if (symbol == '|')
                *type = PIPES;
            return strdup(temp);
        }
    }

    // handle regular tokens (commands and arguments)

    while (**str && !isspace(**str) && **str != '>' && **str != '<' && **str != '|' && **str != '\'' && **str != '"')
        (*str)++;

     size_t len = *str - start;
    char *token_value = malloc(len + 1);
     strncpy(token_value, start, len);
  

    token_value[len] = '\0';

if (token_value[0] == '$')
{           
 
     char *var_name = strdup(token_value + 1);  // remove the $ symbol
     char *expanded_value = expand_variable(var_name, env);
    free(var_name);

    if (expanded_value)
    {
           
        free(token_value);   
        token_value = expanded_value;  // use the expanded value
    }
    else
    { 
      
        free(token_value);
        token_value = strdup("");  // return an empty string if variable not found
    }
    *type = EXPANDABLE;
}

else
{
    *type = WORD;
}
 
    return token_value;
}

/***********************/
