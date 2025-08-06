/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_child.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 15:03:43 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 17:11:33 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	setup_left_child_pipe(int pipe_fd[2], t_shell *child_shell,
		t_env *env_list, t_ast *root_ast)
{
	t_cleanup_ctx	ctx;

	if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
	{
		perror("dup2 left child stdout");
		ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, 1};
		child_cleanup_and_exit(&ctx);
	}
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	set_signal_mode(SIGMODE_CHILD, NULL);
}

void	execute_command_in_child(t_ast *cmd_node,
		t_env *env_list, t_shell *child_shell, t_ast *root_ast)
{
	t_cmd			cmd;
	t_cleanup_ctx	ctx;

	ft_bzero(&cmd, sizeof(t_cmd));
	cmd.args = cmd_node->args;
	if (!child_shell)
	{
		ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, 1};
		child_cleanup_and_exit(&ctx);
	}
	execute_command_common_child(env_list, &cmd, child_shell, root_ast);
}

void	handle_left_child_node(t_ast *left, t_env *env_list,
		t_shell *child_shell, t_ast *root_ast)
{
	t_env			*env_copy;
	int				result;
	t_cleanup_ctx	ctx;

	if (left->type == NODE_COMMAND)
	{
		if (setup_command_redirections(left, &env_list, child_shell) != 0)
		{
			ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, 1};
			child_cleanup_and_exit(&ctx);
		}
		execute_command_in_child(left, env_list, child_shell, root_ast);
	}
	else if (left->type == NODE_REDIR)
	{
		env_copy = env_list;
		result = execute_redirection(left, &env_copy, child_shell);
		ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, result};
		child_cleanup_and_exit(&ctx);
	}
	else
	{
		ctx = (t_cleanup_ctx){child_shell, env_list, NULL, root_ast, 1};
		child_cleanup_and_exit(&ctx);
	}
}
