#include "minishell.h"

int main(void)
{
    char *input;
    t_token **tokens;

    while (1)
    {
        setup_signal_handlers();
        input = readline("minishell$ ");
        if (!input) // CTRL+D durumu
            break;
        
        if (*input)
            add_history(input);
        
        tokens = lexer(input);
        print_tokens(tokens);
        
        free(tokens);
        free(input);
    }
    return 0;
}
