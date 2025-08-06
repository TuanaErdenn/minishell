/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/20 20:09:05 by terden            #+#    #+#             */
/*   Updated: 2025/08/03 14:07:53 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "libft/libft.h"
# include <errno.h>
# include <fcntl.h>
# include <signal.h>
# include <stdio.h>
# include <readline/history.h>
# include <readline/readline.h>
# include <stdlib.h>
# include <string.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <unistd.h>

extern volatile sig_atomic_t	g_interrupt_flag;

typedef enum e_token_type
{
	T_WORD,
	T_PIPE,
	T_INPUT,
	T_OUTPUT,
	T_APPEND,
	T_HEREDOC,
	T_INVALID
}					t_token_type;

typedef enum e_quote_type
{
	Q_NONE,
	Q_SINGLE,
	Q_DOUBLE
}					t_quote_type;

typedef enum e_node_type
{
	NODE_COMMAND,
	NODE_PIPE,
	NODE_REDIR
}					t_node_type;

typedef enum e_redirect_type
{
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC
}					t_redirect_type;

typedef enum e_sigmode
{
	SIGMODE_PROMPT,
	SIGMODE_HEREDOC,
	SIGMODE_CHILD,
	SIGMODE_NEUTRAL
}					t_sigmode;

typedef struct s_token
{
	char			*value;
	t_token_type	type;
	t_quote_type	quote_type;
	int				was_expanded;
	struct s_token	*next;
	struct s_token	*prev;
}					t_token;

typedef struct s_redirection
{
	t_token_type	type;
	char			*filename;
	t_quote_type	quote_type;
	int				position;
	int				fd;
	char			*temp_file;
	int				is_applied;
	char			*delimiter;
	int				should_expand;
}					t_redirection;

typedef struct s_ast
{
	t_node_type		type;
	char			**args;
	t_quote_type	*quote_types;
	t_redirect_type	redirect_type;
	char			*file;
	t_quote_type	file_quote;
	struct s_ast	*left;
	struct s_ast	*right;
	t_redirection	*redirections;
	int				redir_count;
	int				saved_stdin_fd;
	int				saved_stdout_fd;
	int				redirections_applied;
	char			**input_files;
	char			**output_files;
	char			**append_files;
	char			**heredoc_delims;
	char			**heredoc_contents;
	t_quote_type	*input_quotes;
	t_quote_type	*output_quotes;
	t_quote_type	*append_quotes;
	t_quote_type	*heredoc_quotes;
	int				input_count;
	int				output_count;
	int				append_count;
	int				heredoc_count;
}					t_ast;

typedef struct s_env
{
	char			*key;
	char			*value;
	struct s_env	*next;
}					t_env;

typedef struct t_shell
{
	int				exit_code;
	char			**envp;
	int				saved_stdin;
	int				saved_stdout;
	int				heredoc_counter;
	int				interrupted;
	t_ast			*ast;
	char			**heredoc_temp_files;
	int				heredoc_temp_count;
	t_env			*env_list;
	int				heredoc_fd;
	int				needs_heredoc_cleanup;
	int				should_exit;
}					t_shell;

typedef struct s_pipe_ctx
{
	t_ast		*pipe_node;
	t_env		*env_list;
	t_ast		*root_ast;
	t_shell		*shell;
}	t_pipe_ctx;

typedef struct s_cleanup_ctx
{
	t_shell		*shell;
	t_env		*env_list;
	char		**envp;
	t_ast		*original_ast;
	int			exit_code;
}	t_cleanup_ctx;

typedef struct s_cmd
{
	char			**args;
	int				is_pipe;
	int				has_input_redir;
	int				has_output_redir;
	int				has_append_redir;
	int				has_heredoc;
	int				has_dollar;
	int				q_type;
	int				*args_quote_type;
	char			*input_file;
	char			*output_file;
	char			*append_file;
	char			*heredoc_delim;
}					t_cmd;

typedef struct s_token_ctx
{
	char		*input;
	int			i;
	int			start;
	int			quote_type;
	int			single_quote_count;
	int			double_quote_count;
	int			should_expand;
	int			only_var_expand;
	char		*result;
	t_token		**tokens;
	int			*token_index;
	t_shell		*shell;
}	t_token_ctx;

typedef struct s_cmd_info
{
	t_ast	*cmd;
	t_token	**tokens;
	int		i;
	int		arg_i;
	int		redir_i;
	int		pos;
}	t_cmd_info;

typedef struct s_child_ctx
{
	t_ast		*node;
	t_env		*env_list;
	t_ast		*root_ast;
	t_shell		*parent_shell;
	int			pipe_fd[2];
}				t_child_ctx;

int					setup_command_redirections(t_ast *cmd, t_env **env_list,
						t_shell *shell);
int					restore_command_redirections(t_ast *cmd);
void				cleanup_heredoc_temp_files(t_shell *shell);
void				free_heredoc_list(t_shell *shell);
int					apply_redirection(t_redirection *redir, t_ast *cmd,
						t_shell *shell);
char				*create_heredoc_temp_file(t_shell *shell);
t_token				**tokenize_with_expansion(char *input, t_shell *shell);
t_token				*create_token(char *value, t_token_type type);
t_token				*create_token_with_quote(char *value, t_token_type type,
						int quote_type);
t_token				*create_token_with_expansion(char *value, t_token_type type,
						int quote_type, int was_expanded);
void				freetokens(t_token **tokens);
t_ast				*parse_tokens(t_token **tokens);
t_ast				*parse_pipe(t_token **tokens, int pipe_index);
t_ast				*parse_command(t_token **tokens, int start, int end);
void				free_ast(t_ast *node);
void				execute_ast(t_ast *node, t_env **env_list, t_shell *shell);
char				*get_env_value(t_env *env_list, char *key);
t_env				*init_env_list(char **environ);
void				free_env_list(t_env *env_list);
char				**env_to_array(t_env *env_list);
void				free_env_array(char **envp);
int					add_update_env(t_env **env_list, char *key, char *value);
int					execute_export(t_env **env_list, char **args);
char				*merge_export_tokens(char **args, int start_index,
						int *merged_count);
int					parse_export_arg(char *arg, char **key,
						char **value);
char				*remove_quotes(char *str);
void				print_sorted_env(t_env **env_list);
int					process_export_arg(t_env **env_list, char **args,
						int i, int *status);
int					is_valid_identifier(char *key);
int					execute_unset(t_env **env_list, char **args);
void				add_env_node(t_env **env_list, t_env *new_node);
t_env				*new_env_node(char *key, char *value);
t_env				*create_env_node(char *key, char *value);
void				split_env_line(char *line, char **key, char **value);
int					ft_pwd(t_env *env_list);
int					ft_echo(t_env *env_list, char **args, t_cmd cmd,
						t_shell *shell);
int					ft_env(t_env *env_list);
int					ft_cd(t_env *env_list, char **args);
int					builtin_exit(char **args, t_shell *shell);
int					is_builtin(t_cmd *cmd);
int					run_builtin(t_env *env_list, t_cmd *cmd, t_shell *shell);
int					count_args(char **args);
char				*get_cd_path(t_env *env_list, char **args);
int					execute_command_common(t_env *env_list, t_cmd *cmd,
						t_shell *shell);
int					execute_external_command(t_cmd *cmd, t_shell *shell);
int					execute_pipe(t_ast *pipe_node, t_env *env_list,
						t_shell *shell, t_ast *root_ast);
void				validate_child_command(t_cmd *cmd, t_shell *shell,
						t_env *env_list, t_ast *root_ast);
void				close_pipes_safe(int *pipe_fd, int count);
int					handle_pipe_signals(int right_status, t_shell *shell);
void				copy_heredoc_info_to_child(t_shell *parent_shell,
						t_shell *child_shell);
void				handle_left_child_node(t_ast *left, t_env *env_list,
						t_shell *child_shell, t_ast *root_ast);
void				execute_command_in_child(t_ast *cmd_node,
						t_env *env_list, t_shell *child_shell, t_ast *root_ast);
void				setup_left_child_pipe(int pipe_fd[2], t_shell *child_shell,
						t_env *env_list, t_ast *root_ast);
int					execute_command_common_child(t_env *env_list, t_cmd *cmd,
						t_shell *shell, t_ast *root_ast);
void				init_child_shell(t_shell *child_shell,
						t_shell *parent_shell);
void				setup_right_child_pipe(int pipe_fd[2], t_shell *child_shell,
						t_env *env_list, t_ast *root_ast);
void				handle_right_child_node(t_ast *right, t_env *env_list,
						t_shell *child_shell, t_ast *root_ast);
void				execute_c_process(char *path, t_cmd *cmd,
						t_env *env_list, t_shell *shell);
char				*get_executable_path(char *cmd_name, t_env *env_list,
						t_shell *shell);
int					handle_process_status(int status, t_shell *shell);
int					check_path_validity(char *path, t_shell *shell);
void				execute_child_builtin(t_env *env_list, t_cmd *cmd,
						t_shell *shell, t_ast *root_ast);
void				execute_child_external(t_env *env_list, t_cmd *cmd,
						t_shell *shell, t_ast *root_ast);
pid_t				fork_left_child(t_pipe_ctx *ctx, int pipe_fd[2]);
pid_t				fork_right_child(t_pipe_ctx *ctx, int pipe_fd[2],
						pid_t left_pid);
int					check_left_and_fork_right(t_pipe_ctx *ctx, int pipe_fd[2],
						pid_t left_pid, pid_t *right_pid);
char				*expand_string_with_vars(const char *str, t_shell *shell);
char				*handle_var_expansion(char **result, const char *str,
						int *i, t_shell *shell);
char				*handle_dollar_expansion(char **result,
						const char *str, int *i, t_shell *shell);
char				*add_char_to_result(char **result, char c);
int					expand_variable(t_env *env_list, char *str, int start);
void				print_with_expansion(t_env *env_list, char *str,
						t_shell *shell);
int					execute_redirection(t_ast *redir_node, t_env **env_list,
						t_shell *shell);
int					restore_redirections(t_shell *shell);
char				*find_exec(char *command, t_env *env_list);
int					ft_strcmp(const char *s1, const char *s2);
char				*ft_strncpy(char *dest, const char *src, size_t n);
int					is_n_flag(const char *str);
int					handle_exit(char *input, t_env *env_list);
void				free_key_value(char *key, char *value);
void				set_signal_mode(t_sigmode mode, t_shell *shell);
int					check_and_reset_signal(t_shell *shell);
void				handle_sigint_parent(int sig);
void				handle_sigint_child(int sig);
void				handle_sigint_heredoc(int sig);
void				handle_sigquit_child(int sig);
void				free_heredoc(t_shell *shell);
int					check_quotes(char *input);
void				process_input(char *input, t_env *env_list, t_shell *shell);
void				initshell(t_shell *shell, char **envp);
int					is_space(char c);
int					is_break_char(char c);
int					handle_quotes_and_vars(t_token_ctx *ctx, int temp_i);
void				set_quote_type(t_token_ctx *ctx);
char				*expand_and_join(t_token_ctx *ctx, char *str);
char				*join_and_free(t_token_ctx *ctx, char *str);
void				init_token_ctx(t_token_ctx *ctx);
int					create_word_token_enhanced(t_token_ctx *ctx);
int					scan_input(t_token_ctx *ctx);
int					finalize_token(t_token_ctx *ctx);
int					append_quoted(t_token_ctx *ctx, int j);
int					append_unquoted(t_token_ctx *ctx, int j);
int					skip_spaces(char *input, int i);
int					create_special_token(char *input, int i,
						t_token **tokens, int *token_index);
void				reset_token_ctx(t_token_ctx *ctx);
int					count_tokens_enhanced(char *input);
int					fill_tokens_enhanced_with_expansion(char *input,
						t_token **tokens, t_shell *shell);
t_ast				*create_ast_node(t_node_type type);
int					fill_cmd(t_ast *cmd, t_token **tokens, int start, int end);
int					init_cmd_data(t_ast *cmd, int arg_count, int redir_count);
void				free_args_array(char **args);
void				setup_left_child_pipe(int pipe_fd[2], t_shell *child_shell,
						t_env *env_list, t_ast *root_ast);
void				execute_command_in_child(t_ast *cmd_node,
						t_env *env_list, t_shell *child_shell, t_ast *root_ast);
void				handle_left_child_node(t_ast *left, t_env *env_list,
						t_shell *child_shell, t_ast *root_ast);
void				execute_left_child_ctx(t_child_ctx *ctx);
void				execute_right_child_ctx(t_child_ctx *ctx);
pid_t				fork_right_child(t_pipe_ctx *ctx,
						int pipe_fd[2], pid_t left_pid);
void				child_cleanup_and_exit(t_cleanup_ctx *ctx);
int					validate_input_redirection(const char *filename);
int					find_last_input_redirection(t_ast *cmd);
int					find_last_output_redirection(t_ast *cmd);
int					save_and_dup_stdin(t_ast *cmd, t_shell *shell, int fd);
int					apply_input_redirection(t_redirection *redir,
						t_ast *cmd, t_shell *shell);
int					apply_output_redirection(t_redirection *redir,
						t_ast *cmd, t_shell *shell);
int					apply_append_redirection(t_redirection *redir,
						t_ast *cmd, t_shell *shell);
void				set_heredoc_delimiter_and_expansion(t_redirection *redir);
int					add_temp_file_to_list(t_shell *shell,
						const char *temp_file);
int					setup_heredoc_child(t_shell *shell, const char *temp_file);
void				heredoc_input_loop(int fd, const char *delimiter,
						int should_expand, t_shell *shell);
void				cleanup_heredoc_child(t_shell *shell, t_env **env_list);
int					handle_child_exit_status(int status,
						t_shell *shell, const char *temp_file);
void				heredoc_input_loop(int fd, const char *delimiter,
						int should_expand, t_shell *shell);
int					read_heredoc(const char *delimiter, int should_expand,
						t_shell *shell, const char *temp_file);
int					process_heredoc_delimiter(t_redirection *redir,
						t_shell *shell);
int					preprocess_all_heredocs(t_ast *ast, t_shell *shell);
t_ast				*create_empty_cmd_node(void);
#endif