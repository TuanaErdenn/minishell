/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_4.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 13:09:35 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/30 16:47:52 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	execute_child_builtin(t_env *env_list, t_cmd *cmd,
		t_shell *shell, t_ast *root_ast)
{
	int				result;
	t_cleanup_ctx	ctx;

	result = run_builtin(env_list, cmd, shell);
	ctx = (t_cleanup_ctx){shell, env_list, NULL, root_ast, result};
	child_cleanup_and_exit(&ctx);
}

void	execute_child_external(t_env *env_list, t_cmd *cmd,
		t_shell *shell, t_ast *root_ast)
{
	char			**envp;
	char			*path;
	t_cleanup_ctx	ctx;

	path = find_exec(cmd->args[0], env_list);
	if (!path)
	{
		write(STDERR_FILENO, "minishell: ", 11);
		write(STDERR_FILENO, cmd->args[0], ft_strlen(cmd->args[0]));
		write(STDERR_FILENO, ": command not found\n", 20);
		ctx = (t_cleanup_ctx){shell, env_list, NULL, root_ast, 127};
		child_cleanup_and_exit(&ctx);
	}
	envp = env_to_array(env_list);
	if (!envp)
	{
		free(path);
		ctx = (t_cleanup_ctx){shell, env_list, NULL, root_ast, 1};
		child_cleanup_and_exit(&ctx);
	}
	execve(path, cmd->args, envp);
	perror("execve");
	free(path);
	ctx = (t_cleanup_ctx){shell, env_list, envp, root_ast, 126};
	child_cleanup_and_exit(&ctx);
}

pid_t	fork_left_child(t_pipe_ctx *ctx, int pipe_fd[2])
{
	pid_t		left_pid;
	t_child_ctx	child_ctx;

	left_pid = fork();
	if (left_pid == -1)
	{
		perror("fork left");
		close_pipes_safe(pipe_fd, 2);
		ctx->shell->exit_code = 1;
		return (-1);
	}
	if (left_pid == 0)
	{
		set_signal_mode(SIGMODE_CHILD, NULL);
		child_ctx.node = ctx->pipe_node->left;
		child_ctx.env_list = ctx->env_list;
		child_ctx.root_ast = ctx->root_ast;
		child_ctx.parent_shell = ctx->shell;
		child_ctx.pipe_fd[0] = pipe_fd[0];
		child_ctx.pipe_fd[1] = pipe_fd[1];
		execute_left_child_ctx(&child_ctx);
	}
	return (left_pid);
}
