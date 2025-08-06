/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_5.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 15:11:46 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 16:58:00 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	child_cleanup_and_exit(t_cleanup_ctx *ctx)
{
	if (ctx->shell && ctx->shell->heredoc_temp_files)
	{
		free_heredoc_list(ctx->shell);
	}
	if (ctx->original_ast)
		free_ast(ctx->original_ast);
	if (ctx->env_list)
		free_env_list(ctx->env_list);
	if (ctx->envp)
		free_env_array(ctx->envp);
	exit(ctx->exit_code);
}

static int	handle_external_child(pid_t pid, char *path, t_cmd *cmd,
	t_shell *shell)
{
	int	status;

	if (pid == 0)
		execute_c_process(path, cmd, shell->env_list, shell);
	if (path != cmd->args[0])
		free(path);
	set_signal_mode(SIGMODE_NEUTRAL, shell);
	waitpid(pid, &status, 0);
	set_signal_mode(SIGMODE_PROMPT, shell);
	return (handle_process_status(status, shell));
}

int	execute_external_command(t_cmd *cmd, t_shell *shell)
{
	pid_t	pid;
	char	*path;

	if (!cmd || !cmd->args || !cmd->args[0] || !*(cmd->args[0]))
	{
		shell->exit_code = 0;
		return (0);
	}
	path = get_executable_path(cmd->args[0], shell->env_list, shell);
	if (!path)
		return (shell->exit_code);
	pid = fork();
	if (pid == -1)
	{
		perror("fork");
		if (path != cmd->args[0])
			free(path);
		shell->exit_code = 1;
		return (1);
	}
	return (handle_external_child(pid, path, cmd, shell));
}

static int	print_path_error(const char *msg, char *path,
	int code, t_shell *shell)
{
	write(STDERR_FILENO, "minishell: ", 11);
	write(STDERR_FILENO, path, ft_strlen(path));
	write(STDERR_FILENO, msg, ft_strlen(msg));
	shell->exit_code = code;
	return (code);
}

int	check_path_validity(char *path, t_shell *shell)
{
	struct stat	file_stat;

	if (access(path, F_OK) != 0)
		return (print_path_error(": No such file or directory\n",
				path, 127, shell));
	if (stat(path, &file_stat) == 0 && S_ISDIR(file_stat.st_mode))
		return (print_path_error(": Is a directory\n", path, 126, shell));
	if (access(path, X_OK) != 0)
		return (print_path_error(": Permission denied\n", path, 126, shell));
	return (0);
}
