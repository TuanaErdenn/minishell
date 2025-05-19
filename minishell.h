#ifndef MINISHELL_H
# define MINISHELL_H



# include "libft/libft.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <readline/readline.h>  // readline fonksiyonu için 
# include <readline/history.h>  // add_history için


typedef enum e_token_type {
    T_WORD,       // Genel kelime veya arg
    T_PIPE,       // |
    T_INPUT,      // <
    T_OUTPUT,     // >
    T_APPEND,     // >>
    T_HEREDOC,   // <<
    T_QUOTE       // ' veya "
}   t_token_type; //asagıdakini bunla kullan


typedef struct s_token {
    char *value;           // Token metni
    t_token_type type;     // Token tipi
	int quote_type;        // 0: no quote, 1: single quote, 2: double quote

} t_token;

typedef struct s_astnode
{
    char                *content;
    struct s_astnode    *left;
    struct s_astnode    *right;
}   t_astnode;

typedef struct s_cmd {
    char **args;
    int is_pipe;
    int has_input_redir;
    int has_output_redir;
    int has_append_redir;
    int has_heredoc;
    int has_dollar;
    int q_type;
    int *args_quote_type;  // Her argümanın tırnak tipini saklamak için yeni alan
    char *input_file;
    char *output_file;
    char *append_file;
    char *heredoc_delim;
} t_cmd;

typedef struct t_shell
{
    int     exit_code;      // Son komutun exit kodu
    char    **envp;         // Environment variables
} t_shell;

typedef struct s_env
{
    char *key;// Değişken adı (ör: "USER")
    char *value;         // Değişken değeri (ör: "ahmet")
    struct s_env *next;  // Listedeki sonraki düğüm
} t_env;

int ft_pwd(void);
int ft_strcmp(const char *s1, const char *s2);
int ft_echo(t_env *env_list, char **args, t_cmd cmd, int exit_code);
t_token **tokenize(char *input);
void print_env(char **envp);
int execute_command(t_env *env_list, t_cmd *cmd, int exit_code);
int is_builtin(t_cmd *cmd);
void freetokens(t_token **tokens);
void	free_token(t_token *token);
t_token	*create_token(char *value, t_token_type type);
char *get_env_value(t_env *env_list, char *key);
t_env	*init_env_list(char **environ);
void free_env_list(t_env *env_list);
int run_builtin(t_env *env_list, t_cmd *cmd, int exit_code);
int ft_env(t_env *env_list);
int ft_cd(t_env *env_list, char **args);
// builtin.c dosyasının başına ekleyin
void print_with_expansion(t_env *env_list, char *str, int exit_code);

t_astnode *create_ast_node(char *content);
void free_ast(t_astnode *node);
t_astnode *parse(t_token **tokens);

#endif