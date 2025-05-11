#include "minishell.h"

/* Lexer'ı Başlat */
t_lexer *init_lexer(char *input)
{
    t_lexer *lexer = malloc(sizeof(t_lexer));
    if (!lexer)
        return (NULL);
    lexer->state = Q0;
    lexer->start = 0;
    lexer->end = 0;
    lexer->input = input;
    return lexer;
}

/* Durum Güncelleme */
void update_state(t_lexer *lexer, char c)
{
    if (lexer->state == Q0)
    {
        if (c == '\'')
            lexer->state = Q1;
        else if (c == '"')
            lexer->state = Q2;
    }
    else if (lexer->state == Q1 && c == '\'')
        lexer->state = Q0;
    else if (lexer->state == Q2 && c == '"')
        lexer->state = Q0;
}

// Tek Tırnak İçeriğini İşleme - düzeltilmiş versiyon */
void handle_single_quotes(t_lexer *lexer, t_token **tokens, int *index)
{
    lexer->start = lexer->end + 1;  // `'` karakterini geç
    lexer->end++;  // Başlangıç pozisyonunu güncelle

    while (lexer->input[lexer->end] && lexer->input[lexer->end] != '\'')
        lexer->end++;

    // Eğer sonuna geldiysek ve kapanış tırnağı bulamadıysak
    if (lexer->input[lexer->end] == '\0')
    {
        printf("Syntax Error: Unclosed single quote\n");
        char *value = strndup(&lexer->input[lexer->start - 1], lexer->end - lexer->start + 1);
        if (value)
        {
            tokens[*index] = create_token(value, SYNTAX_ERROR);
            (*index)++;
            free(value);
        }
    }
    else  // Kapanış tırnağı bulundu
    {
        char *value = strndup(&lexer->input[lexer->start], lexer->end - lexer->start);
        if (value)
        {
            tokens[*index] = create_token(value, SINGLE_QUOTE);
            (*index)++;
            free(value);
        }
        lexer->end++;  // Kapanış tırnağını geç
    }

    lexer->start = lexer->end;  // Sonraki token için başlangıcı ayarla
}

// Çift Tırnak İçeriğini İşleme
void handle_double_quotes(t_lexer *lexer, t_token **tokens, int *index)
{
    lexer->start = lexer->end + 1;  // `"` karakterini geç
    lexer->end++;  // Başlangıç pozisyonunu güncelle

    while (lexer->input[lexer->end] && lexer->input[lexer->end] != '"')
        lexer->end++;

    // Eğer sonuna geldiysek ve kapanış tırnağı bulamadıysak
    if (lexer->input[lexer->end] == '\0')
    {
        printf("Syntax Error: Unclosed double quote\n");
        char *value = strndup(&lexer->input[lexer->start - 1], lexer->end - lexer->start + 1);
        if (value)
        {
            tokens[*index] = create_token(value, SYNTAX_ERROR);
            (*index)++;
            free(value);
        }
    }
    else  // Kapanış tırnağı bulundu
    {
        char *value = strndup(&lexer->input[lexer->start], lexer->end - lexer->start);
        if (value)
        {
            tokens[*index] = create_token(value, DOUBLE_QUOTE);
            (*index)++;
            free(value);
        }
        lexer->end++;  // Kapanış tırnağını geç
    }

    lexer->start = lexer->end;  // Sonraki token için başlangıcı ayarla
}

//Lexer Fonksiyonu 
t_token **lexer(char *input)
{
    t_lexer *lexer = init_lexer(input);
    t_token **tokens = malloc(sizeof(t_token *) * 100);
    int i = 0;

    while (lexer->input[lexer->end])
    {
        update_state(lexer, lexer->input[lexer->end]);

        if (lexer->state == Q0)
        {
            if (lexer->input[lexer->end] == ' ')
            {
                add_token(lexer, tokens, &i, WORD);
                lexer->start = lexer->end + 1;
            }
            else if (lexer->input[lexer->end] == '|')
            {
                add_token(lexer, tokens, &i, WORD);
                tokens[i++] = create_token("|", PIPE);
                lexer->start = lexer->end + 1;
            }
            else if (lexer->input[lexer->end] == '"')
            {
                handle_double_quotes(lexer, tokens, &i);
                continue; // handle fonksiyonu zaten end'i ilerletti, döngüyü atla
            }
            else if (lexer->input[lexer->end] == '\'')
            {
                handle_single_quotes(lexer, tokens, &i);
                continue; // handle fonksiyonu zaten end'i ilerletti, döngüyü atla
            }
        }

        lexer->end++;
    }

    if (lexer->end > lexer->start)
        add_token(lexer, tokens, &i, WORD);

    tokens[i] = NULL;
    free(lexer);
    return tokens;
}