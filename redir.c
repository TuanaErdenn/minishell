/* ✅ CLEAN redir.c - Minimal bash-compliant redirection system */
#include "minishell.h"
#include <sys/stat.h>
#include <errno.h>

/* ==================== NEW REDIRECTION SYSTEM ==================== */

/* ✅ YENİ: Geçici heredoc dosyası oluştur */
char *create_heredoc_temp_file(t_shell *shell)
{
	char *counter_str;
	char *temp_file;
	
	counter_str = ft_itoa(shell->heredoc_counter++);
	if (!counter_str)
		return NULL;
		
	temp_file = ft_strjoin("/tmp/minishell_heredoc_", counter_str);
	free(counter_str);
	
	return temp_file;
}

/* ✅ YENİ: Heredoc içeriğini oku ve geçici dosyaya yaz */
int read_heredoc_content(const char *delimiter, int should_expand, 
                        t_env **env_list, t_shell *shell, const char *temp_file)
{
	char *line;
	char *expanded_line;
	int fd;
	pid_t child_pid;
	int status;
	
	/* Fork ile heredoc okuma işlemini yap */
	child_pid = fork();
	if (child_pid == -1)
	{
		perror("fork heredoc");
		return -1;
	}
	
	if (child_pid == 0)
	{
		/* Child process - heredoc oku */
		set_signal_mode(SIGMODE_HEREDOC, shell);
		
		fd = open(temp_file, O_WRONLY | O_CREAT | O_TRUNC, 0600);
		if (fd == -1)
		{
			perror("heredoc temp file");
			exit(1);
		}
		
		while (1)
		{
			line = readline("> ");
			if (!line)
			{
				/* EOF reached */
				printf("\nminishell: warning: here-document delimited by end-of-file (wanted `%s')\n", delimiter);
				break;
			}
			
			/* Delimiter kontrolü */
			if (ft_strcmp(line, delimiter) == 0)
			{
				free(line);
				break;
			}
			
			/* Variable expansion */
			if (should_expand)
			{
				expanded_line = expand_string_with_vars(line, *env_list, shell);
				if (expanded_line)
				{
					write(fd, expanded_line, ft_strlen(expanded_line));
					free(expanded_line);
				}
				else
				{
					write(fd, line, ft_strlen(line));
				}
			}
			else
			{
				write(fd, line, ft_strlen(line));
			}
			write(fd, "\n", 1);
			
			free(line);
		}
		
		close(fd);
		cleanup_readline();
		
		/* ✅ CRITICAL: Child process memory cleanup to prevent leaks */
		if (*env_list)
		{
			free_env_list(*env_list);
			*env_list = NULL;
		}
		if (shell->ast)
		{
			free_ast(shell->ast);
			shell->ast = NULL;
		}
		if (shell->heredoc_temp_files)
		{
			cleanup_heredoc_temp_files(shell);
		}
		
		exit(0);
	}
	else
	{
		/* Parent process - child'ı bekle */
		waitpid(child_pid, &status, 0);
		
		/* Signal kontrolü */
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		{
			shell->interrupted = 1;
			unlink(temp_file);
			return -1;
		}
		
		if (WEXITSTATUS(status) != 0)
		{
			unlink(temp_file);
			return -1;
		}
	}
	
	return 0;
}

/* ✅ YENİ: Tek bir heredoc redirection'ı işle */
int process_heredoc_delimiter(t_redirection *redir, t_env **env_list, t_shell *shell)
{
	char *expanded_delimiter;
	
	if (!redir || redir->type != T_HEREDOC)
		return 0;
	
	/* Delimiter'ı expand et (quote tipine göre) */
	if (redir->quote_type == Q_SINGLE)
	{
		/* Single quote - expansion yok */
		redir->delimiter = ft_strdup(redir->filename);
		redir->should_expand = 0;
	}
	else
	{
		/* Unquoted veya double quoted - expansion var */
		expanded_delimiter = expand_string_with_vars(redir->filename, *env_list, shell);
		redir->delimiter = expanded_delimiter ? expanded_delimiter : ft_strdup(redir->filename);
		redir->should_expand = 1;
	}
	
	if (!redir->delimiter)
		return -1;
	
	/* Geçici dosya oluştur */
	redir->temp_file = create_heredoc_temp_file(shell);
	if (!redir->temp_file)
	{
		free(redir->delimiter);
		redir->delimiter = NULL;
		return -1;
	}
	
	/* Heredoc içeriğini oku */
	if (read_heredoc_content(redir->delimiter, redir->should_expand, 
	                        env_list, shell, redir->temp_file) != 0)
	{
		free(redir->delimiter);
		free(redir->temp_file);
		redir->delimiter = NULL;
		redir->temp_file = NULL;
		return -1;
	}
	
	/* Shell'e geçici dosyayı kaydet (cleanup için) */
	if (!shell->heredoc_temp_files)
	{
		shell->heredoc_temp_files = malloc(sizeof(char*) * 2);
		if (shell->heredoc_temp_files)
		{
			shell->heredoc_temp_files[0] = ft_strdup(redir->temp_file);
			shell->heredoc_temp_files[1] = NULL;
			shell->heredoc_temp_count = 1;
		}
	}
	else
	{
		/* Mevcut listeyi genişlet */
		char **new_list = realloc(shell->heredoc_temp_files, 
		                         sizeof(char*) * (shell->heredoc_temp_count + 2));
		if (new_list)
		{
			shell->heredoc_temp_files = new_list;
			shell->heredoc_temp_files[shell->heredoc_temp_count] = ft_strdup(redir->temp_file);
			shell->heredoc_temp_files[shell->heredoc_temp_count + 1] = NULL;
			shell->heredoc_temp_count++;
		}
	}
	
	return 0;
}

/* ✅ YENİ: Tüm AST'te heredoc'ları önceden işle (BASH-COMPLIANT) */
int preprocess_all_heredocs(t_ast *ast, t_env **env_list, t_shell *shell)
{
	if (!ast)
		return 0;
	
	/* ✅ CRITICAL: Bu fonksiyon hiçbir child process'te ÇALIŞMAMALI */
	/* Sadece main process'te, fork'tan ÖNCE çalışmalı */
	
	if (ast->type == NODE_COMMAND)
	{
		/* Bu komuttaki tüm heredoc'ları işle */
		for (int i = 0; i < ast->redir_count; i++)
		{
			if (ast->redirections[i].type == T_HEREDOC)
			{
				if (process_heredoc_delimiter(&ast->redirections[i], env_list, shell) != 0)
				{
					/* Heredoc başarısız - cleanup ve exit */
					cleanup_heredoc_temp_files(shell);
					return -1;
				}
			}
		}
	}
	else if (ast->type == NODE_PIPE)
	{
		/* ✅ PIPE: Sol ve sağ tarafı ayrı ayrı işle */
		if (preprocess_all_heredocs(ast->left, env_list, shell) != 0)
			return -1;
		if (preprocess_all_heredocs(ast->right, env_list, shell) != 0)
			return -1;
	}
	else if (ast->type == NODE_REDIR)
	{
		/* Legacy redirection node - ignore for now */
		if (preprocess_all_heredocs(ast->left, env_list, shell) != 0)
			return -1;
	}
	
	return 0;
}

/* ✅ YENİ: Tek bir redirection'ı uygula */
int apply_redirection(t_redirection *redir, t_ast *cmd, t_shell *shell)
{
	int fd;
	
	if (!redir || redir->is_applied)
		return 0;
	
	switch (redir->type)
	{
		case T_INPUT:
			/* Dosya okuma */
			if (access(redir->filename, F_OK) != 0)
			{
				perror(redir->filename);
				return -1;
			}
			if (access(redir->filename, R_OK) != 0)
			{
				perror(redir->filename);
				return -1;
			}
			
			fd = open(redir->filename, O_RDONLY);
			if (fd == -1)
			{
				perror(redir->filename);
				return -1;
			}
			
			/* STDIN'i backup'la (sadece ilk kez) */
			if (cmd->saved_stdin_fd == -1 && shell->saved_stdin != -2)
			{
				cmd->saved_stdin_fd = dup(STDIN_FILENO);
				if (cmd->saved_stdin_fd == -1)
				{
					close(fd);
					perror("dup stdin");
					return -1;
				}
			}
			
			/* STDIN'i yönlendir */
			if (dup2(fd, STDIN_FILENO) == -1)
			{
				close(fd);
				perror("dup2 stdin");
				return -1;
			}
			
			close(fd);
			redir->is_applied = 1;
			break;
			
		case T_OUTPUT:
			/* Dosya yazma (truncate) */
			fd = open(redir->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd == -1)
			{
				perror(redir->filename);
				return -1;
			}
			
			/* STDOUT'u backup'la (sadece ilk kez) */
			if (cmd->saved_stdout_fd == -1 && shell->saved_stdin != -2)
			{
				cmd->saved_stdout_fd = dup(STDOUT_FILENO);
				if (cmd->saved_stdout_fd == -1)
				{
					close(fd);
					perror("dup stdout");
					return -1;
				}
			}
			
			/* STDOUT'u yönlendir */
			if (dup2(fd, STDOUT_FILENO) == -1)
			{
				close(fd);
				perror("dup2 stdout");
				return -1;
			}
			
			close(fd);
			redir->is_applied = 1;
			break;
			
		case T_APPEND:
			/* Dosya yazma (append) */
			fd = open(redir->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (fd == -1)
			{
				perror(redir->filename);
				return -1;
			}
			
			/* STDOUT'u backup'la (sadece ilk kez) */
			if (cmd->saved_stdout_fd == -1 && shell->saved_stdin != -2)
			{
				cmd->saved_stdout_fd = dup(STDOUT_FILENO);
				if (cmd->saved_stdout_fd == -1)
				{
					close(fd);
					perror("dup stdout");
					return -1;
				}
			}
			
			/* STDOUT'u yönlendir */
			if (dup2(fd, STDOUT_FILENO) == -1)
			{
				close(fd);
				perror("dup2 stdout");
				return -1;
			}
			
			close(fd);
			redir->is_applied = 1;
			break;
			
		case T_HEREDOC:
			/* Heredoc geçici dosyasından okuma */
			if (!redir->temp_file)
			{
				printf("Error: Heredoc not preprocessed\n");
				return -1;
			}
			
			fd = open(redir->temp_file, O_RDONLY);
			if (fd == -1)
			{
				perror("heredoc temp file");
				return -1;
			}
			
			/* STDIN'i backup'la (sadece ilk kez) */
			if (cmd->saved_stdin_fd == -1 && shell->saved_stdin != -2)
			{
				cmd->saved_stdin_fd = dup(STDIN_FILENO);
				if (cmd->saved_stdin_fd == -1)
				{
					close(fd);
					perror("dup stdin");
					return -1;
				}
			}
			
			/* STDIN'i yönlendir */
			if (dup2(fd, STDIN_FILENO) == -1)
			{
				close(fd);
				perror("dup2 stdin");
				return -1;
			}
			
			close(fd);
			redir->is_applied = 1;
			break;
			
		default:
			return 0;
	}
	
	return 0;
}

/* ✅ YENİ: Komut için redirection'ları uygula (BASH ORDER) */
int setup_command_redirections(t_ast *cmd, t_env **env_list, t_shell *shell)
{
    (void) env_list;
	if (!cmd || cmd->type != NODE_COMMAND)
		return 0;
	
	/* Initialize backup fds */
	if (cmd->saved_stdin_fd == -1)
		cmd->saved_stdin_fd = -1;
	if (cmd->saved_stdout_fd == -1)
		cmd->saved_stdout_fd = -1;
	cmd->redirections_applied = 0;
	
	/* ✅ BASH BEHAVIOR: Son input ve son output redirection'ı bul */
	int last_input_idx = -1;
	int last_output_idx = -1;
	
	/* Son input redirection'ı bul (INPUT ve HEREDOC dahil) */
	for (int i = cmd->redir_count - 1; i >= 0; i--)
	{
		if (cmd->redirections[i].type == T_INPUT || cmd->redirections[i].type == T_HEREDOC)
		{
			last_input_idx = i;
			break;
		}
	}
	
	/* Son output redirection'ı bul (OUTPUT ve APPEND dahil) */
	for (int i = cmd->redir_count - 1; i >= 0; i--)
	{
		if (cmd->redirections[i].type == T_OUTPUT || cmd->redirections[i].type == T_APPEND)
		{
			last_output_idx = i;
			break;
		}
	}
	
	/* ✅ BASH CRITICAL: Önce tüm output dosyalarını oluştur/kontrol et */
	for (int i = 0; i < cmd->redir_count; i++)
	{
		if (cmd->redirections[i].type == T_OUTPUT)
		{
			/* Test et - dosya oluşturulabilir mi? */
			int test_fd = open(cmd->redirections[i].filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (test_fd == -1)
			{
				perror(cmd->redirections[i].filename);
				return -1; /* Immediate failure */
			}
			close(test_fd);
		}
		else if (cmd->redirections[i].type == T_APPEND)
		{
			/* Test et - dosya açılabilir mi? */
			int test_fd = open(cmd->redirections[i].filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (test_fd == -1)
			{
				perror(cmd->redirections[i].filename);
				return -1; /* Immediate failure */
			}
			close(test_fd);
		}
	}
	
	/* ✅ BASH BEHAVIOR: Sadece son input redirection'ı uygula */
	if (last_input_idx != -1)
	{
		if (apply_redirection(&cmd->redirections[last_input_idx], cmd, shell) != 0)
			return -1;
	}
	
	/* ✅ BASH BEHAVIOR: Sadece son output redirection'ı uygula */
	if (last_output_idx != -1)
	{
		if (apply_redirection(&cmd->redirections[last_output_idx], cmd, shell) != 0)
			return -1;
	}
	
	cmd->redirections_applied = 1;
	return 0;
}

/* ✅ YENİ: Komut redirection'larını geri yükle */
int restore_command_redirections(t_ast *cmd)
{
	if (!cmd || !cmd->redirections_applied)
		return 0;
	
	/* STDIN'i geri yükle */
	if (cmd->saved_stdin_fd != -1)
	{
		if (dup2(cmd->saved_stdin_fd, STDIN_FILENO) == -1)
			perror("restore stdin");
		close(cmd->saved_stdin_fd);
		cmd->saved_stdin_fd = -1;
	}
	
	/* STDOUT'u geri yükle */
	if (cmd->saved_stdout_fd != -1)
	{
		if (dup2(cmd->saved_stdout_fd, STDOUT_FILENO) == -1)
			perror("restore stdout");
		close(cmd->saved_stdout_fd);
		cmd->saved_stdout_fd = -1;
	}
	
	/* Redirection uygulama durumunu sıfırla */
	for (int i = 0; i < cmd->redir_count; i++)
		cmd->redirections[i].is_applied = 0;
	
	cmd->redirections_applied = 0;
	return 0;
}

/* ✅ YENİ: Tüm heredoc geçici dosyalarını temizle */
void cleanup_heredoc_temp_files(t_shell *shell)
{
	if (!shell || !shell->heredoc_temp_files)
		return;
	
	for (int i = 0; i < shell->heredoc_temp_count; i++)
	{
		if (shell->heredoc_temp_files[i])
		{
			unlink(shell->heredoc_temp_files[i]);
			free(shell->heredoc_temp_files[i]);
		}
	}
	
	free(shell->heredoc_temp_files);
	shell->heredoc_temp_files = NULL;
	shell->heredoc_temp_count = 0;
}

/* ==================== LEGACY COMPATIBILITY ==================== */

/* ✅ LEGACY: Backward compatibility wrapper */
int setup_redirections(t_ast *cmd, t_env **env_list, t_shell *shell)
{
	/* Yeni sistemi kullan */
	return setup_command_redirections(cmd, env_list, shell);
}

/* ✅ LEGACY: Global stdin/stdout restore */
int restore_redirections(t_shell *shell)
{
	int result = 0;
	
	if (!shell)
		return -1;
	
	/* Restore stdin */
	if (shell->saved_stdin != -1)
	{
		if (dup2(shell->saved_stdin, STDIN_FILENO) == -1)
		{
			perror("dup2 restore stdin");
			result = -1;
		}
		close(shell->saved_stdin);
		shell->saved_stdin = -1;
	}
	
	/* Restore stdout */
	if (shell->saved_stdout != -1)
	{
		if (dup2(shell->saved_stdout, STDOUT_FILENO) == -1)
		{
			perror("dup2 restore stdout");
			result = -1;
		}
		close(shell->saved_stdout);
		shell->saved_stdout = -1;
	}
	
	return result;
}

/* ✅ LEGACY: Simple redirection execution (kept for compatibility) */
int execute_redirection(t_ast *redir_node, t_env **env_list, t_shell *shell)
{
	if (!redir_node || !shell)
		return 1;

	/* Find the actual command node */
	t_ast *cmd_node = redir_node;
	while (cmd_node && cmd_node->type == NODE_REDIR)
		cmd_node = cmd_node->left;
	
	if (!cmd_node)
		return 1;
	
	/* Execute the command */
	int result = 0;
	if (cmd_node->type == NODE_PIPE)
	{
		result = execute_pipe(cmd_node, *env_list, shell, cmd_node);
	}
	else
	{
		execute_ast(cmd_node, env_list, shell);
		result = shell->exit_code;
	}
	
	/* Restore redirections */
	restore_redirections(shell);
	
	return result;
}