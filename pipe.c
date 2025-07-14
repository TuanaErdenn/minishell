/* Düzeltilmiş pipe.c - Array-based redirection support */
#include "minishell.h"
#include <signal.h>

/* Pipe'ları güvenli şekilde kapatma */
static void close_pipes_safe(int *pipe_fd, int count)
{
	int i = 0;
	while (i < count)
	{
		if (pipe_fd[i] != -1)
		{
			close(pipe_fd[i]);
			pipe_fd[i] = -1;
		}
		i++;
	}
}

/* Child process cleanup - exit'ten önce belleği temizle */
static void child_cleanup_and_exit(t_env *env_list, char **envp, t_ast *original_ast, int exit_code)
{
	if (original_ast)
		free_ast(original_ast);

	if (env_list)
		free_env_list(env_list);
	if (envp)
		free_env_array(envp);
	exit(exit_code);
}

// Child process için özel memory cleanup ile
static int execute_command_common_child(t_env *env_list, t_cmd *cmd, t_shell *shell, t_ast *root_ast)
{
	char **envp = NULL;
	char *path = NULL;
	int result = 0;

	if (!cmd || !cmd->args || !cmd->args[0] || !*(cmd->args[0]))
	{
		// Boş komut - cleanup and exit
		if (cmd->args_quote_type)
			free(cmd->args_quote_type);
		child_cleanup_and_exit(env_list, NULL, root_ast, 0);
	}

	if (is_builtin(cmd))
	{
		result = run_builtin(env_list, cmd, shell);
		
		// Builtin çalıştırıldı - cleanup and exit
		if (cmd->args_quote_type)
			free(cmd->args_quote_type);
		child_cleanup_and_exit(env_list, NULL, root_ast, result);
	}
	else
	{
		// External command - execve ile çalıştır
		path = find_exec(cmd->args[0], env_list);
		if (!path)
		{
			write(STDERR_FILENO, "minishell: ", 11);
			write(STDERR_FILENO, cmd->args[0], ft_strlen(cmd->args[0]));
			write(STDERR_FILENO, ": command not found\n", 20);
			
			// Command not found - cleanup and exit
			if (cmd->args_quote_type)
				free(cmd->args_quote_type);
			child_cleanup_and_exit(env_list, NULL, root_ast, 127);
		}

		envp = env_to_array(env_list);
		if (!envp)
		{
			free(path);
			if (cmd->args_quote_type)
				free(cmd->args_quote_type);
			child_cleanup_and_exit(env_list, NULL, root_ast, 1);
		}

		// execve çağrısından önce malloc'lanmış memory'yi temizle
		if (cmd->args_quote_type)
		{
			free(cmd->args_quote_type);
			cmd->args_quote_type = NULL;
		}

		// execve çağrısı - başarılı olursa zaten process değişir
		execve(path, cmd->args, envp);

		// execve failed - cleanup and exit
		perror("execve");
		free(path);
		child_cleanup_and_exit(env_list, envp, root_ast, 126);
	}

	return (0);  // Bu satıra hiç gelmeyecek
}

static void execute_command_in_child(t_ast *cmd_node, t_env *env_list, t_shell *child_shell, t_ast *root_ast)
{
	t_cmd cmd;
	int arg_count = 0;

	ft_bzero(&cmd, sizeof(t_cmd));
	cmd.args = cmd_node->args;
	cmd.args_quote_type = NULL;  // Initialize to NULL

	if (!child_shell)
	{
		child_cleanup_and_exit(env_list, NULL, root_ast, 1);
	}

	/* Setup redirections */
	if (setup_redirections(cmd_node, &env_list, child_shell) != 0)
	{
		child_cleanup_and_exit(env_list, NULL, root_ast, 1);
	}

	/* Quote types handling */
	if (cmd_node->quote_types)
	{
		// Count arguments first
		while (cmd_node->args && cmd_node->args[arg_count])
			arg_count++;

		if (arg_count > 0)
		{
			cmd.args_quote_type = malloc(sizeof(int) * arg_count);
			if (cmd.args_quote_type)
			{
				for (int i = 0; i < arg_count; i++)
					cmd.args_quote_type[i] = (int)cmd_node->quote_types[i];
			}
			else
			{
				// Malloc failed - cleanup and exit
				child_cleanup_and_exit(env_list, NULL, root_ast, 1);
			}
		}
	}

	/* ✅ FIXED: Execute command with proper memory cleanup */
	execute_command_common_child(env_list, &cmd, child_shell, root_ast);

	/* Bu noktaya hiç gelmeyecek çünkü execute_command_common_child exit() çağırır */
}

int execute_command_common(t_env *env_list, t_cmd *cmd, t_shell *shell, int is_child_process)
{
	int result;

	if (!cmd || !cmd->args || !cmd->args[0] || !*(cmd->args[0]))
	{
		if (is_child_process)
		{
			// Child process - cleanup and exit
			if (cmd && cmd->args_quote_type)
				free(cmd->args_quote_type);
			exit(0);
		}
		return (0); // Boş komut
	}

	if (is_builtin(cmd))
	{
		result = run_builtin(env_list, cmd, shell);

		if (is_child_process)
		{
			// Child process ise cleanup and exit
			if (cmd->args_quote_type)
				free(cmd->args_quote_type);
			exit(result);
		}
		else
		{
			// Parent process ise sadece result döndür
			return (result);
		}
	}
	else
	{
		// External command handling
		if (is_child_process)
		{
			// Child process - execve ile çalıştır
			char *path = find_exec(cmd->args[0], env_list);
			if (!path)
			{
				write(STDERR_FILENO, "minishell: ", 11);
				write(STDERR_FILENO, cmd->args[0], ft_strlen(cmd->args[0]));
				write(STDERR_FILENO, ": command not found\n", 20);
				
				// Command not found - cleanup and exit
				if (cmd->args_quote_type)
					free(cmd->args_quote_type);
				exit(127);
			}

			char **envp = env_to_array(env_list);
			if (!envp)
			{
				free(path);
				if (cmd->args_quote_type)
					free(cmd->args_quote_type);
				exit(1);
			}

			// execve çağrısından önce malloc'lanmış memory'yi temizle
			if (cmd->args_quote_type)
			{
				free(cmd->args_quote_type);
				cmd->args_quote_type = NULL;
			}

			execve(path, cmd->args, envp);

			// execve failed - cleanup and exit
			perror("execve");
			free(path);
			free_env_array(envp);
			exit(126);
		}
		else
		{
			// Parent process - fork ve execute
			result = execute_external_command(env_list, cmd, shell);
			return (result);
		}
	}

	return (0);
}

int execute_external_command(t_env *env_list, t_cmd *cmd, t_shell *shell)
{
	pid_t pid;
	int status;
	char *path = NULL;
	struct stat file_stat;

	// Boş komut kontrolü
	if (!cmd || !cmd->args || !cmd->args[0] || !*(cmd->args[0]))
	{
		shell->exit_code = 0;
		return (0);
	}

	// Check if it's a path (contains '/')
	if (ft_strchr(cmd->args[0], '/'))
	{
		// It's a path - check if file exists first
		if (access(cmd->args[0], F_OK) != 0)
		{
			// File doesn't exist
			write(STDERR_FILENO, "minishell: ", 11);
			write(STDERR_FILENO, cmd->args[0], ft_strlen(cmd->args[0]));
			write(STDERR_FILENO, ": No such file or directory\n", 28);
			shell->exit_code = 127;
			return (127);
		}

		// Check if it's a directory
		if (stat(cmd->args[0], &file_stat) == 0 && S_ISDIR(file_stat.st_mode))
		{
			// It's a directory
			write(STDERR_FILENO, "minishell: ", 11);
			write(STDERR_FILENO, cmd->args[0], ft_strlen(cmd->args[0]));
			write(STDERR_FILENO, ": Is a directory\n", 17);
			shell->exit_code = 126;
			return (126);
		}

		// Check if it's executable
		if (access(cmd->args[0], X_OK) != 0)
		{
			// File exists but not executable
			write(STDERR_FILENO, "minishell: ", 11);
			write(STDERR_FILENO, cmd->args[0], ft_strlen(cmd->args[0]));
			write(STDERR_FILENO, ": Permission denied\n", 20);
			shell->exit_code = 126;
			return (126);
		}

		path = cmd->args[0]; // Use the path directly
	}
	else
	{
		// Search in PATH
		path = find_exec(cmd->args[0], env_list);
		if (!path)
		{
			write(STDERR_FILENO, "minishell: ", 11);
			write(STDERR_FILENO, cmd->args[0], ft_strlen(cmd->args[0]));
			write(STDERR_FILENO, ": command not found\n", 20);
			shell->exit_code = 127;
			return (127);
		}
	}

	// Fork process
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		if (path != cmd->args[0])
			free(path);
		shell->exit_code = 1;
		return (1);
	}

	if (pid == 0)
	{
		// CHILD PROCESS
		set_signal_mode(SIGMODE_CHILD, NULL);

		// Prepare environment array
		char **envp = env_to_array(env_list);
		if (!envp)
			exit(1);

		// Execute command
		execve(path, cmd->args, envp);

		// execve failed
		perror("execve");
		free_env_array(envp);
		exit(126);
	}

	// PARENT PROCESS
	if (path != cmd->args[0])
		free(path);

	waitpid(pid, &status, 0);
	
	set_signal_mode(SIGMODE_PROMPT, shell);

	// Debug: status değerini kontrol et
	if (status == 2) // SIGINT ile öldürüldü
	{
		write(STDOUT_FILENO, "\n", 1);
		shell->exit_code = 130;
		return (130);
	}
	else if (status == 0)
	{
		shell->exit_code = 0;
		return (0);
	}
	else if (status > 255)
	{
		shell->exit_code = status >> 8;
		return (shell->exit_code);
	}
	else
	{
		shell->exit_code = status;
		return (status);
	}
}

/* ✅ FIXED: Sol child process (pipe'ın sol tarafı) */
static void execute_left_child(t_ast *left, t_env *env_list, int pipe_fd[2], t_ast *root_ast)
{
	// ✅ PROPER shell initialization - FIX for missing field initializers
	t_shell child_shell;
	child_shell.exit_code = 0;
	child_shell.envp = NULL;
	child_shell.saved_stdin = -1;
	child_shell.saved_stdout = -1;
	child_shell.heredoc_counter = 0;

	// STDOUT'u pipe'a yönlendir
	if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
	{
		perror("dup2 left child");
		child_cleanup_and_exit(env_list, NULL, root_ast, 1);
	}

	// Pipe'ları kapat
	close(pipe_fd[0]);
	close(pipe_fd[1]);

	if (left->type == NODE_COMMAND)
	{
		execute_command_in_child(left, env_list, &child_shell, root_ast);
	}
	else if (left->type == NODE_REDIR)
	{
		// ✅ OLD SYSTEM: Legacy redirection node support
		t_env *env_copy = env_list;
		int result = execute_redirection(left, &env_copy, &child_shell);
		child_cleanup_and_exit(env_list, NULL, root_ast, result);
	}

	child_cleanup_and_exit(env_list, NULL, root_ast, 1);
}

/* ✅ FIXED: Sağ child process (pipe'ın sağ tarafı) */
static void execute_right_child(t_ast *right, t_env *env_list, int pipe_fd[2], t_ast *root_ast)
{
	// ✅ PROPER shell initialization - FIX for missing field initializers
	t_shell child_shell;
	child_shell.exit_code = 0;
	child_shell.envp = NULL;
	child_shell.saved_stdin = -1;
	child_shell.saved_stdout = -1;
	child_shell.heredoc_counter = 0;

	// STDIN'i pipe'dan al
	if (dup2(pipe_fd[0], STDIN_FILENO) == -1)
	{
		perror("dup2 right child");
		child_cleanup_and_exit(env_list, NULL, root_ast, 1);
	}

	// Pipe'ları kapat
	close(pipe_fd[0]);
	close(pipe_fd[1]);

	if (right->type == NODE_COMMAND)
	{
		execute_command_in_child(right, env_list, &child_shell, root_ast);
	}
	else if (right->type == NODE_PIPE)
	{
		// Nested pipe
		execute_pipe(right, env_list, &child_shell, root_ast);
		child_cleanup_and_exit(env_list, NULL, root_ast, child_shell.exit_code);
	}
	else if (right->type == NODE_REDIR)
	{
		// ✅ OLD SYSTEM: Legacy redirection node support
		t_env *env_copy = env_list;
		int result = execute_redirection(right, &env_copy, &child_shell);
		child_cleanup_and_exit(env_list, NULL, root_ast, result);
	}

	child_cleanup_and_exit(env_list, NULL, root_ast, 1);
}

/* Ana pipe execution fonksiyonu */
int execute_pipe(t_ast *pipe_node, t_env *env_list, t_shell *shell, t_ast *root_ast)
{
	int pipe_fd[2];
	pid_t left_pid, right_pid;
	int left_status, right_status;

	// Pipe oluştur
	if (pipe(pipe_fd) == -1)
	{
		perror("pipe");
		shell->exit_code = 1;
		return (1);
	}

	// Sol child process
	left_pid = fork();
	if (left_pid == -1)
	{
		perror("fork left");
		close_pipes_safe(pipe_fd, 2);
		shell->exit_code = 1;
		return (1);
	}

	if (left_pid == 0)
	{
		set_signal_mode(SIGMODE_CHILD, NULL);
		// Sol child - hiç return etmez, exit ile çıkar
		execute_left_child(pipe_node->left, env_list, pipe_fd, root_ast);
	}

	// Sağ child process
	right_pid = fork();
	if (right_pid == -1)
	{
		perror("fork right");
		close_pipes_safe(pipe_fd, 2);
		kill(left_pid, 15); // SIGTERM yerine doğrudan sayı kullan
		waitpid(left_pid, NULL, 0);
		shell->exit_code = 1;
		return (1);
	}

	if (right_pid == 0)
	{
		set_signal_mode(SIGMODE_CHILD, NULL);
		// Sağ child - hiç return etmez, exit ile çıkar
		execute_right_child(pipe_node->right, env_list, pipe_fd, root_ast);
	}

	// Parent process: pipe'ları kapat ve child'ları bekle
	close_pipes_safe(pipe_fd, 2);

	// Her iki child'ın da bitmesini bekle
	waitpid(left_pid, &left_status, 0);
	waitpid(right_pid, &right_status, 0);

	// SIGINT durumunda newline yaz ve 130 dön
	if (left_status == 2 || right_status == 2)
	{
		write(STDOUT_FILENO, "\n", 1);
		shell->exit_code = 130;
		return (130);
	}
	
	// Normal exit - sağ child'ın exit code'unu kullan
	if (right_status == 0)
	{
		shell->exit_code = 0;
		return (0);
	}
	else if (right_status > 255)
	{
		shell->exit_code = right_status >> 8;
		return (shell->exit_code);
	}
	else
	{
		shell->exit_code = right_status;
		return (right_status);
	}
}
