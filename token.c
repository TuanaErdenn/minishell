#include "minishell.h"

/* Yeni Bir Token Oluştur */
t_token *create_token(char *value, t_token_type type)
{
    t_token *token = malloc(sizeof(t_token));
    if (!token)
        return (NULL);
    token->value = strdup(value);
    token->type = type;
    return (token);
}

/* Yeni Token Ekleme */
void add_token(t_lexer *lexer, t_token **tokens, int *index, t_token_type type)
{
    if (lexer->end > lexer->start)
    {
        char *value = strndup(&lexer->input[lexer->start], lexer->end - lexer->start);
        tokens[*index] = create_token(value, type);
        free(value);
        (*index)++;
    }
    lexer->start = lexer->end + 1;
}

/* Tokenleri Yazdır */
void print_tokens(t_token **tokens)
{
    int i = 0;
    while (tokens[i])
    {
        printf("Token %d: %s (%d)\n", i, tokens[i]->value, tokens[i]->type);
        free(tokens[i]->value);
        free(tokens[i]);
        i++;
    }
}
