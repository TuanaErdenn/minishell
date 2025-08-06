/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_child_exec.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 15:03:25 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 16:48:59 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	execute_left_child_ctx(t_child_ctx *ctx)
{
	t_shell			child_shell;
	t_cleanup_ctx	cleanup_ctx;

	init_child_shell(&child_shell, ctx->parent_shell);
	copy_heredoc_info_to_child(ctx->parent_shell, &child_shell);
	setup_left_child_pipe(ctx->pipe_fd, &child_shell,
		ctx->env_list, ctx->root_ast);
	handle_left_child_node(ctx->node, ctx->env_list,
		&child_shell, ctx->root_ast);
	cleanup_ctx = (t_cleanup_ctx){&child_shell, ctx->env_list,
		NULL, ctx->root_ast, 1};
	child_cleanup_and_exit(&cleanup_ctx);
}

void	execute_right_child_ctx(t_child_ctx *ctx)
{
	t_shell			child_shell;
	t_cleanup_ctx	cleanup_ctx;

	init_child_shell(&child_shell, ctx->parent_shell);
	copy_heredoc_info_to_child(ctx->parent_shell, &child_shell);
	setup_right_child_pipe(ctx->pipe_fd, &child_shell,
		ctx->env_list, ctx->root_ast);
	handle_right_child_node(ctx->node, ctx->env_list,
		&child_shell, ctx->root_ast);
	cleanup_ctx = (t_cleanup_ctx){&child_shell, ctx->env_list,
		NULL, ctx->root_ast, 1};
	child_cleanup_and_exit(&cleanup_ctx);
}

static void	handle_fork_failure(pid_t left_pid,
		int pipe_fd[2], t_pipe_ctx *ctx)
{
	perror("fork right");
	close_pipes_safe(pipe_fd, 2);
	kill(left_pid, SIGTERM);
	waitpid(left_pid, NULL, 0);
	ctx->shell->exit_code = 1;
}

pid_t	fork_right_child(t_pipe_ctx *ctx, int pipe_fd[2], pid_t left_pid)
{
	pid_t		right_pid;
	t_child_ctx	child_ctx;

	right_pid = fork();
	if (right_pid == -1)
	{
		handle_fork_failure(left_pid, pipe_fd, ctx);
		return (-1);
	}
	if (right_pid == 0)
	{
		child_ctx.node = ctx->pipe_node->right;
		child_ctx.env_list = ctx->env_list;
		child_ctx.root_ast = ctx->root_ast;
		child_ctx.parent_shell = ctx->shell;
		child_ctx.pipe_fd[0] = pipe_fd[0];
		child_ctx.pipe_fd[1] = pipe_fd[1];
		execute_right_child_ctx(&child_ctx);
	}
	return (right_pid);
}
