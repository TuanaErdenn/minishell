/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 13:09:05 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/30 17:18:52 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

#include "minishell.h"

void	init_child_shell(t_shell *child_shell, t_shell *parent_shell)
{
	ft_bzero(child_shell, sizeof(t_shell));
	child_shell->saved_stdin = -2;
	child_shell->saved_stdout = -1;
	copy_heredoc_info_to_child(parent_shell, child_shell);
}

void	setup_right_child_pipe(int pipe_fd[2], t_shell *child_shell,
		t_env *env_list, t_ast *root_ast)
{
	t_cleanup_ctx	ctx;

	if (dup2(pipe_fd[0], STDIN_FILENO) == -1)
	{
		perror("dup2 right child stdin");
		ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, 1};
		child_cleanup_and_exit(&ctx);
	}
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	set_signal_mode(SIGMODE_CHILD, NULL);
}

static void	handle_redir_or_error(t_ast *right, t_env *env_list,
	t_shell *child_shell, t_ast *root_ast)
{
	t_env			*env_copy;
	int				result;
	t_cleanup_ctx	ctx;

	if (right->type == NODE_REDIR)
	{
		env_copy = env_list;
		result = execute_redirection(right, &env_copy, child_shell);
		ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, result};
		child_cleanup_and_exit(&ctx);
	}
	else
	{
		printf("ERROR: Unhandled right node type: %d\n", right->type);
		ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, 1};
		child_cleanup_and_exit(&ctx);
	}
}

void	handle_right_child_node(t_ast *right, t_env *env_list,
	t_shell *child_shell, t_ast *root_ast)
{
	t_cleanup_ctx	ctx;
	int				result;

	if (right->type == NODE_COMMAND)
	{
		if (setup_command_redirections(right, &env_list, child_shell) != 0)
		{
			ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, 1};
			child_cleanup_and_exit(&ctx);
		}
		execute_command_in_child(right, env_list, child_shell, root_ast);
	}
	else if (right->type == NODE_PIPE)
	{
		result = execute_pipe(right, env_list, child_shell, root_ast);
		ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, result};
		child_cleanup_and_exit(&ctx);
	}
	else
		handle_redir_or_error(right, env_list, child_shell, root_ast);
}

int	handle_process_status(int status, t_shell *shell)
{
	if (status == 2)
	{
		write(STDOUT_FILENO, "\n", 1);
		shell->exit_code = 130;
		return (130);
	}
	if (status == 3 || status == 33280 || status == 131)
	{
		write(STDOUT_FILENO, "Quit (core dumped)\n", 19);
		shell->exit_code = 131;
		return (131);
	}
	if (status == 0)
	{
		shell->exit_code = 0;
		return (0);
	}
	if (status > 255)
	{
		shell->exit_code = status >> 8;
		return (shell->exit_code);
	}
	shell->exit_code = status;
	return (status);
}
