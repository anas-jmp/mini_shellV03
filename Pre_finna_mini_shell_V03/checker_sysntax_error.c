#include "mini_shell.h"

int check_unmatched_quotes(char *line)
{
    // unmatched_quotes !!!!
 
    char quote = 0;
    int i = 0;
    while( line[i])
     {
        //printf("\n%d\n",line[i]); nb.: ascii value
        if (line[0] == '\'' || line[0] == '"' || line[0]=='>' ||line[0]=='<')
        {
           printf("Syntax error: Unmatched quotes\n");
             return 1;
        }
        if (line[i] == '\'' || line[i] == '"')
        {
            if (quote == 0)
            {
                quote = line[i];  // Open quote
            } 
            else if (quote == line[i])
             {
                quote = 0;  // Close quote
            }
        }
        i++;
    }
                if(quote != 0)  // Return 1 if there's an unmatched quote
                {
                    printf("Syntax error: Unmatched quotes\n");
                     return 1;
                }
                else
                    return 0;

}
 
int check_unmatched_redirection(char *line)
{
    int i = 0;
    while (line[i])
    {
        if (line[i] == '<' || line[i] == '>')
        {
            // Skip spaces
            i++;
            while (line[i] == ' ') 
            {
                i++;
            }
            // Check if the next character is a valid file name character
            if (line[i] == '\0' || line[i] == '<' || line[i] == '>' || line[i] == '|') 
            {
                printf("Syntax error: Invalid redirection\n");
                return 1;
            }
        }
        i++;
    }
        return 0;
}

int check_unmatched_pipes(char *line)
 {
    int i = 0;
    // Skip initial spaces
    while (line[i] == ' ') 
    {
        i++;
    }
    if (line[i] == '|')
    {
        printf("Syntax error:  pipe\n");
        return 1;
    }

    int last_char_was_pipe = 0;
    while (line[i]) 
    {
        if (line[i] == '|')
        {
            if (last_char_was_pipe)
            {
                printf("Syntax error: Consecutive pipes\n");
                return 1;
            }
            last_char_was_pipe = 1;
        } 
        else if (line[i] != ' ')
        {
            last_char_was_pipe = 0;
        }
        i++;
    }

    // Check if the last character is a pipe
    i--;
    while (i >= 0 && line[i] == ' ')
    {
        i--;
    }
    if (line[i] == '|')
    {
        printf("Syntax error: Misplaced pipe\n");
          return 1;
    }
    else
    return 0;
}