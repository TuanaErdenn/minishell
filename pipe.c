/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 13:08:26 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/08/01 17:52:51 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	finalize_pipe_execution(int left_status, int right_status,
		t_shell *shell)
{
	int	signal_result;

	(void)left_status;
	signal_result = handle_pipe_signals(right_status, shell);
	if (signal_result != 0)
	{
		shell->needs_heredoc_cleanup = 1;
		return (signal_result);
	}
	shell->needs_heredoc_cleanup = 1;
	if (WIFEXITED(right_status))
	{
		shell->exit_code = WEXITSTATUS(right_status);
		return (shell->exit_code);
	}
	shell->exit_code = 1;
	return (1);
}

static int	setup_pipe_and_left_child(t_pipe_ctx *ctx, int pipe_fd[2],
		pid_t *left_pid)
{
	if (pipe(pipe_fd) == -1)
	{
		perror("pipe");
		ctx->shell->exit_code = 1;
		return (1);
	}
	*left_pid = fork_left_child(ctx, pipe_fd);
	if (*left_pid == -1)
		return (1);
	return (0);
}

int	check_left_and_fork_right(t_pipe_ctx *ctx, int pipe_fd[2],
		pid_t left_pid, pid_t *right_pid)
{
	*right_pid = fork_right_child(ctx, pipe_fd, left_pid);
	if (*right_pid == -1)
		return (1);
	return (0);
}

static int	wait_and_finalize(pid_t left_pid, pid_t right_pid,
		int pipe_fd[2], t_shell *shell)
{
	int	left_status;
	int	right_status;

	close_pipes_safe(pipe_fd, 2);
	waitpid(left_pid, &left_status, 0);
	waitpid(right_pid, &right_status, 0);
	return (finalize_pipe_execution(left_status, right_status, shell));
}

int	execute_pipe(t_ast *pipe_node, t_env *env_list, t_shell *shell,
		t_ast *root_ast)
{
	t_pipe_ctx	ctx;
	int			pipe_fd[2];
	pid_t		left_pid;
	pid_t		right_pid;
	int			result;

	ctx.pipe_node = pipe_node;
	ctx.env_list = env_list;
	ctx.root_ast = root_ast;
	ctx.shell = shell;
	result = setup_pipe_and_left_child(&ctx, pipe_fd, &left_pid);
	if (result != 0)
		return (result);
	result = check_left_and_fork_right(&ctx, pipe_fd, left_pid, &right_pid);
	if (result != 0)
		return (result);
	return (wait_and_finalize(left_pid, right_pid, pipe_fd, shell));
}
