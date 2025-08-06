/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_1.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 13:08:48 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/30 16:42:34 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	close_pipes_safe(int *pipe_fd, int count)
{
	int	i;

	i = 0;
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

void	validate_child_command(t_cmd *cmd, t_shell *shell,
		t_env *env_list, t_ast *root_ast)
{
	t_cleanup_ctx	ctx;

	if (!cmd || !cmd->args || !cmd->args[0])
	{
		ctx = (t_cleanup_ctx){shell, env_list, NULL, root_ast, 0};
		child_cleanup_and_exit(&ctx);
	}
	if (!*(cmd->args[0]))
	{
		write(STDERR_FILENO, "minishell: ", 11);
		write(STDERR_FILENO, ": command not found\n", 20);
		ctx = (t_cleanup_ctx){shell, env_list, NULL, root_ast, 127};
		child_cleanup_and_exit(&ctx);
	}
}

int	handle_pipe_signals(int right_status, t_shell *shell)
{
	int	sig;

	if (WIFSIGNALED(right_status))
	{
		sig = WTERMSIG(right_status);
		if (sig == SIGINT)
		{
			write(STDOUT_FILENO, "\n", 1);
			shell->exit_code = 130;
			return (130);
		}
		if (sig == SIGQUIT)
		{
			write(STDOUT_FILENO, "Quit (core dumped)\n", 19);
			shell->exit_code = 131;
			return (131);
		}
	}
	return (0);
}

void	copy_heredoc_info_to_child(t_shell *parent_shell,
	t_shell *child_shell)
{
	if (!parent_shell->heredoc_temp_files
		|| parent_shell->heredoc_temp_count == 0)
		return ;
	child_shell->heredoc_temp_files = parent_shell->heredoc_temp_files;
	child_shell->heredoc_temp_count = parent_shell->heredoc_temp_count;
}
