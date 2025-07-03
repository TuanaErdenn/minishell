#ifndef MINISHELL_H
# define MINISHELL_H

/* ==================== STANDART KÜTÜPHANELER ==================== */
# include "libft/libft.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <readline/readline.h>  // readline fonksiyonu için 
# include <readline/history.h>  // add_history için
# include <sys/wait.h> //waitpid için
#include <fcntl.h>
#include <signal.h>
/* ==================== ENUM TİPLERİ ==================== */

/* Token türlerini belirtir */
typedef enum e_token_type {
	T_WORD,        // Genel kelime veya arg
	T_PIPE,        // |
	T_INPUT,       // <
	T_OUTPUT,      // >
	T_APPEND,      // >>
	T_HEREDOC,     // <<
	T_INVALID      // Tanımsız veya hatalı token
}	t_token_type;

/* Argüman veya dosya adlarındaki quote tiplerini belirtir */
typedef enum e_quote_type {
	Q_NONE,        // Quote yok
	Q_SINGLE,      // '
	Q_DOUBLE       // "
}	t_quote_type;

/* AST düğüm tipleri */
typedef enum e_node_type {
	NODE_COMMAND,  // Komut düğümü (ls, echo ...)
	NODE_PIPE,     // Pipe (|) düğümü
	NODE_REDIR     // Redirect (<, >, >>, <<) düğümü
}	t_node_type;

/* Redirect yönlerini belirtir */
typedef enum e_redirect_type {
	REDIR_IN,      // <
	REDIR_OUT,     // >
	REDIR_APPEND,  // >>
	REDIR_HEREDOC  // <<
}	t_redirect_type;



/* ==================== TOKEN YAPISI ==================== */

/* Lexing aşamasında oluşturulan token'ları temsil eder */
typedef struct s_token {
	char *value;             // Token metni
	t_token_type type;       // Token tipi
	t_quote_type quote_type; // Quote bilgisi
	struct s_token	*next;   // Token listesi için bağlı liste yapısı

} t_token;

/* ==================== PARSER AST YAPILARI ==================== */

/* Parser tarafından oluşturulan soyut sözdizim ağacı (AST) düğümleri */
typedef struct s_ast {
	t_node_type type;            // Komut, pipe veya redirect
	char            **args;          // Komut argümanları (NODE_COMMAND için)
	t_quote_type	*quote_types;     // Her argümana karşılık gelen quote türleri (args ile paralel)
	t_redirect_type redirect_type;   // Yönlendirme türü (NODE_REDIR için)
	char            *file;           // Yönlendirilen dosya adı (NODE_REDIR için)
	t_quote_type    file_quote;      // Dosya isminin quote bilgisi (özellikle heredoc için önemli)
	struct s_ast    *left;           // Sol alt düğüm
	struct s_ast    *right;          // Sağ alt düğüm
	
	// ✅ NEW: Array-based redirection system
	char **input_files;        // Input redirection dosyaları
    char **output_files;       // Output redirection dosyaları  
    char **append_files;       // Append redirection dosyaları
    char **heredoc_delims;     // Heredoc delimiter'ları
    
    t_quote_type *input_quotes;         // Input dosyalarının quote durumu
    t_quote_type *output_quotes;        // Output dosyalarının quote durumu
    t_quote_type *append_quotes;        // Append dosyalarının quote durumu
    t_quote_type *heredoc_quotes;       // Heredoc delimiter'larının quote durumu
    
    int input_count;
    int output_count;
    int append_count;
    int heredoc_count;
}	t_ast;

/* ==================== KOMUT YAPISI (EXECUTION) ==================== */
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
	
	// ✅ NEW: Redirection için saved file descriptors (global değişken yerine)
	int     saved_stdin;    // Original STDIN_FILENO backup
	int     saved_stdout;   // Original STDOUT_FILENO backup
	int     heredoc_counter; // Heredoc temp file unique counter
} t_shell;

/* ==================== UPDATED FUNCTION PROTOTYPES ==================== */

// ✅ UPDATED: restore_redirections now takes shell parameter
int restore_redirections(t_shell *shell);
/* ==================== ENVIRONMENT YAPISI ==================== */
typedef struct s_env
{
	char *key;           // Değişken adı (ör: "USER")
	char *value;         // Değişken değeri (ör: "ahmet")
	struct s_env *next;  // Listedeki sonraki düğüm
} t_env;

/* ==================== SİNYAL YÖNETİMİ ==================== */
typedef enum e_sigmode
{
	SIGMODE_PROMPT,
	SIGMODE_HEREDOC,
	SIGMODE_CHILD
}	t_sigmode;

void	set_signal_mode(t_sigmode mode, t_shell *shell);

int	builtin_exit(char **args, t_shell *shell);



/* ==================== BUILTIN VE GENEL FONKSİYONLAR ==================== */
int ft_pwd(t_env *env_list);  // ✅ UPDATED: now takes env_list parameter
int ft_strcmp(const char *s1, const char *s2);
int ft_echo(t_env *env_list, char **args, t_cmd cmd, t_shell *shell);
int ft_env(t_env *env_list);
int ft_cd(t_env *env_list, char **args);
// builtin.c dosyasının başına ekleyin
void print_with_expansion(t_env *env_list, char *str, t_shell *shell);
int execute_command(t_env *env_list, t_cmd *cmd, t_shell *shell);
int is_builtin(t_cmd *cmd);
int run_builtin(t_env *env_list, t_cmd *cmd, t_shell *shell);
// Pipe execution function - NEW SIGNATURE
int execute_pipe(t_ast *pipe_node, t_env *env_list, t_shell *shell, t_ast *root_ast);

/* ==================== ENV FONKSİYONLARI ==================== */
char *get_env_value(t_env *env_list, char *key);
t_env	*init_env_list(char **environ);
void free_env_list(t_env *env_list);

/* ==================== TOKENIZER FONKSİYONLARI ==================== */
t_token **tokenize(char *input);
t_token	*create_token(char *value, t_token_type type);
void freetokens(t_token **tokens);
void	free_token(t_token *token);

/* ==================== PARSER FONKSİYONLARI ==================== */

t_ast *parse_tokens(t_token **tokens);
t_ast *parse_pipe(t_token **tokens, int pipe_index);
t_ast *parse_command(t_token **tokens, int start, int end);
void free_ast(t_ast *node);
void execute_ast(t_ast *node, t_env **env_list, t_shell *shell);

/*=========================== EXPANDER ==========================*/
void expand_ast(t_ast *node, t_env *env_list, t_shell *shell);

char **env_to_array(t_env *env_list);
int	add_update_env(t_env **env_list, char *key, char *value);
int	execute_export(t_env **env_list, char **args);
int	execute_unset(t_env **env_list, char **args);
void add_env_node(t_env **env_list, t_env *new_node);
void free_env_array(char **envp);
char *find_exec(char *command, t_env *env_list);

/*===================== REDIRECTION FONKSİYONLARI ======================*/
char *expand_string_with_vars(const char *str, t_env *env_list, t_shell *shell);
int redir_in(t_ast *redir_node, t_env **env_list, t_shell *shell);
int execute_redirection(t_ast *redir_node, t_env **env_list, t_shell *shell);

// ✅ NEW: Array-based redirection functions
int setup_redirections(t_ast *cmd, t_env **env_list, t_shell *shell);
#include <sys/stat.h>
#endif