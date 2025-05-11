#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <signal.h>
# include <unistd.h>
# include <string.h>
# include <readline/readline.h>
# include <readline/history.h>

//Tırnak durum kontrolleri
typedef enum e_state
{
    Q0,  // Tırnaksız
    Q1,  // Tek Tırnak
    Q2   // Çift Tırnak
} t_state;

typedef struct s_lexer
{
    t_state state;   // Durum: Q0, Q1, Q2
    int     start;   // Token başlangıç index'i
    int     end;     // Token bitiş index'i
    char    *input;  // Girdi (orijinal string)
} t_lexer;


/* Token Tipleri */
typedef enum e_token_type
{
    WORD,
    PIPE,
    REDIRECT_IN,
    REDIRECT_OUT,
    REDIRECT_APPEND,
    HEREDOC,
    VARIABLE,
    EXIT_STATUS,
    SINGLE_QUOTE,
    DOUBLE_QUOTE,
    SYNTAX_ERROR
} t_token_type;

/* Token Yapısı */
typedef struct s_token
{
    char            *value;
    t_token_type    type;
} t_token;


// Sinyal işlemleri
void setup_signal_handlers(void);

void update_state(t_lexer *lexer, char c);
t_lexer *init_lexer(char *input);
t_token **lexer(char *input);
void print_tokens(t_token **tokens);
t_token *create_token(char *value, t_token_type type);
void add_token(t_lexer *lexer, t_token **tokens, int *index, t_token_type type);
void handle_double_quotes(t_lexer *lexer, t_token **tokens, int *index);
void handle_single_quotes(t_lexer *lexer, t_token **tokens, int *index);


#endif
