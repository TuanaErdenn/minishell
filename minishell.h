#ifndef MINISHELL_H
# define MINISHELL_H

/* ==================== STANDART KÜTÜPHANELER ==================== */
# include "libft/libft.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <readline/readline.h>
# include <readline/history.h>
# include <sys/wait.h>
# include <fcntl.h>
# include <signal.h>
# include <sys/stat.h>
# include <errno.h>

/* ==================== ENUM TİPLERİ ==================== */

typedef enum e_token_type {
	T_WORD,
	T_PIPE,
	T_INPUT,
	T_OUTPUT,
	T_APPEND,
	T_HEREDOC,
	T_INVALID
}	t_token_type;

typedef enum e_quote_type {
	Q_NONE,
	Q_SINGLE,
	Q_DOUBLE
}	t_quote_type;

typedef enum e_node_type {
	NODE_COMMAND,
	NODE_PIPE,
	NODE_REDIR
}	t_node_type;

typedef enum e_redirect_type {
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC
}	t_redirect_type;

typedef enum e_sigmode
{
	SIGMODE_PROMPT,
	SIGMODE_HEREDOC,
	SIGMODE_CHILD,
	SIGMODE_NEUTRAL
}	t_sigmode;

/* ==================== TOKEN YAPISI ==================== */

typedef struct s_token {
	char *value;
	t_token_type type;
	t_quote_type quote_type;
	struct s_token	*next;
	struct s_token	*prev;
} t_token;

/* ==================== REDIRECTION YAPILARI ==================== */

/* ✅ YENİ: Her redirection için fd ve geçici dosya bilgisi */
typedef struct s_redirection
{
	t_token_type type;          // T_INPUT, T_OUTPUT, T_APPEND, T_HEREDOC
	char *filename;             // Filename for redirection
	t_quote_type quote_type;    // Quote type of filename
	int position;               // Original position in command line
	
	/* ✅ YENİ: FD management */
	int fd;                     // Açılan dosya descriptor'u
	char *temp_file;            // Heredoc için geçici dosya adı
	int is_applied;             // Bu redirection uygulandı mı?
	
	/* ✅ YENİ: Heredoc specific */
	char *delimiter;            // Heredoc delimiter (expanded)
	int should_expand;          // Variable expansion yapılacak mı?
} t_redirection;

/* ==================== AST YAPISI ==================== */
typedef struct s_ast {
	t_node_type type;
	char **args;
	t_quote_type *quote_types;
	t_redirect_type redirect_type;   // Legacy
	char *file;                      // Legacy
	t_quote_type file_quote;         // Legacy
	struct s_ast *left;
	struct s_ast *right;
	
	/* ✅ YENİ: Geliştirilmiş redirection sistemi */
	t_redirection *redirections;     // Sıralı redirection dizisi
	int redir_count;                 // Toplam redirection sayısı
	
	/* ✅ YENİ: Pre-execution fd backup */
	int saved_stdin_fd;              // Bu komut için kaydedilen STDIN
	int saved_stdout_fd;             // Bu komut için kaydedilen STDOUT
	int redirections_applied;        // Redirections uygulandı mı?
	
	/* Legacy arrays - backward compatibility için */
	char **input_files;
	char **output_files;
	char **append_files;
	char **heredoc_delims;
	char **heredoc_contents;
	t_quote_type *input_quotes;
	t_quote_type *output_quotes;
	t_quote_type *append_quotes;
	t_quote_type *heredoc_quotes;
	int input_count;
	int output_count;
	int append_count;
	int heredoc_count;
}	t_ast;

/* ==================== SHELL YAPISI ==================== */
typedef struct t_shell
{
	int exit_code;
	char **envp;
	int saved_stdin;             // Global stdin backup
	int saved_stdout;            // Global stdout backup
	int heredoc_counter;         // Unique heredoc temp file counter
	int interrupted;             // Signal interrupt flag
	t_ast *ast;                  // Root AST pointer
	
	/* ✅ YENİ: Heredoc pre-processing */
	char **heredoc_temp_files;   // Tüm heredoc geçici dosyaları
	int heredoc_temp_count;      // Geçici dosya sayısı
} t_shell;

/* ==================== ENVIRONMENT YAPISI ==================== */
typedef struct s_env
{
	char *key;
	char *value;
	struct s_env *next;
} t_env;

/* ==================== KOMUT YAPISI ==================== */
typedef struct s_cmd {
	char **args;
	int is_pipe;
	int has_input_redir;
	int has_output_redir;
	int has_append_redir;
	int has_heredoc;
	int has_dollar;
	int q_type;
	int *args_quote_type;
	char *input_file;
	char *output_file;
	char *append_file;
	char *heredoc_delim;
} t_cmd;

/* ==================== FUNCTION PROTOTYPES ==================== */

/* ✅ YENİ: Geliştirilmiş redirection fonksiyonları */
int preprocess_all_heredocs(t_ast *ast, t_env **env_list, t_shell *shell);
int setup_command_redirections(t_ast *cmd, t_env **env_list, t_shell *shell);
int restore_command_redirections(t_ast *cmd);
void cleanup_heredoc_temp_files(t_shell *shell);
int apply_redirection(t_redirection *redir, t_ast *cmd, t_shell *shell);

/* ✅ YENİ: Heredoc processing */
int process_heredoc_delimiter(t_redirection *redir, t_env **env_list, t_shell *shell);
char *create_heredoc_temp_file(t_shell *shell);
int read_heredoc_content(const char *delimiter, int should_expand, 
                        t_env **env_list, t_shell *shell, const char *temp_file);

/* Token ve Parser fonksiyonları */
t_token **tokenize(char *input);
t_token **tokenize_with_expansion(char *input, t_env *env_list, t_shell *shell);
t_token *create_token(char *value, t_token_type type);
void freetokens(t_token **tokens);
void free_token(t_token *token);
int fill_tokens_enhanced_with_expansion(char *input, t_token **tokens, t_env *env_list, t_shell *shell);
int fill_tokens_enhanced(char *input, t_token **tokens);
int create_word_token_enhanced_simple(char *input, int i, t_token **tokens, int *token_index);

t_ast *parse_tokens(t_token **tokens);
t_ast *parse_pipe(t_token **tokens, int pipe_index);
t_ast *parse_command(t_token **tokens, int start, int end);
void free_ast(t_ast *node);
void execute_ast(t_ast *node, t_env **env_list, t_shell *shell);

/* Environment fonksiyonları */
char *get_env_value(t_env *env_list, char *key);
t_env *init_env_list(char **environ);
void free_env_list(t_env *env_list);
char **env_to_array(t_env *env_list);
void free_env_array(char **envp);
int add_update_env(t_env **env_list, char *key, char *value);
int execute_export(t_env **env_list, char **args);
int execute_unset(t_env **env_list, char **args);
void add_env_node(t_env **env_list, t_env *new_node);
t_env *new_env_node(char *key, char *value);
void split_env_line(char *line, char **key, char **value);

/* Builtin fonksiyonları */
int ft_pwd(t_env *env_list);
int ft_echo(t_env *env_list, char **args, t_cmd cmd, t_shell *shell);
int ft_env(t_env *env_list);
int ft_cd(t_env *env_list, char **args);
int builtin_exit(char **args, t_shell *shell);
int is_builtin(t_cmd *cmd);
int run_builtin(t_env *env_list, t_cmd *cmd, t_shell *shell);

/* Execution fonksiyonları */
int execute_command_common(t_env *env_list, t_cmd *cmd, t_shell *shell, int is_child_process);
int execute_external_command(t_env *env_list, t_cmd *cmd, t_shell *shell);
int execute_pipe(t_ast *pipe_node, t_env *env_list, t_shell *shell, t_ast *root_ast);

/* Expansion fonksiyonları */
void expand_ast(t_ast *node, t_env *env_list, t_shell *shell);
char *expand_string_with_vars(const char *str, t_env *env_list, t_shell *shell);
char *expand_token_with_vars(t_token *token, t_env *env_list, t_shell *shell);
int expand_variable(t_env *env_list, char *str, int start);
int should_remove_arg(const char *original, const char *expanded, t_quote_type quote_type);
void print_with_expansion(t_env *env_list, char *str, t_shell *shell);

/* Legacy redirection fonksiyonları */
int redir_in(t_ast *redir_node, t_env **env_list, t_shell *shell);
int execute_redirection(t_ast *redir_node, t_env **env_list, t_shell *shell);
int setup_redirections(t_ast *cmd, t_env **env_list, t_shell *shell);
int restore_redirections(t_shell *shell);

/* Utility fonksiyonları */
char *find_exec(char *command, t_env *env_list);
int ft_strcmp(const char *s1, const char *s2);
char *ft_strncpy(char *dest, const char *src, size_t n);
int is_n_flag(const char *str);
void free_str_array(char **arr);
int handle_exit(char *input, t_env *env_list);
void cleanup_readline(void);

/* Signal management */
void set_signal_mode(t_sigmode mode, t_shell *shell);
int check_and_reset_signal(t_shell *shell);
void set_token_prev_fields(t_token **tokens);

#endif