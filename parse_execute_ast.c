/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_execute_ast.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 22:23:49 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 00:44:02 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	free_args_array(char **args)
{
	int	i;

	if (!args)
		return ;
	i = 0;
	while (args[i])
	{
		free(args[i]);
		args[i] = NULL;
		i++;
	}
	free(args);
}

static void	copy_quote_types(t_cmd *cmd, t_ast *current)
{
	int	i;

	i = 0;
	while (current->args[i])
		i++;
	cmd->args_quote_type = malloc(sizeof(int) * i);
	if (!cmd->args_quote_type)
		return ;
	i = 0;
	while (current->args[i])
	{
		cmd->args_quote_type[i] = (int)current->quote_types[i];
		i++;
	}
}

static void	handle_command_node(t_ast *current,
			t_env **env_list, t_shell *shell)
{
	t_cmd	cmd;
	int		redir_result;

	if (preprocess_all_heredocs(current, shell) != 0)
		return ((void)(shell->exit_code = 1),
			cleanup_heredoc_temp_files(shell));
	ft_bzero(&cmd, sizeof(t_cmd));
	cmd.args = current->args;
	if (!cmd.args || !cmd.args[0])
		return ((void)(shell->exit_code = 0),
			cleanup_heredoc_temp_files(shell));
	if (current->quote_types && current->args)
		copy_quote_types(&cmd, current);
	else
		cmd.args_quote_type = NULL;
	redir_result = setup_command_redirections(current, env_list, shell);
	if (redir_result != 0)
		return ((void)(shell->exit_code = 1), free(cmd.args_quote_type),
			cleanup_heredoc_temp_files(shell));
	shell->exit_code = execute_command_common(*env_list, &cmd, shell);
	restore_command_redirections(current);
	free(cmd.args_quote_type);
	cleanup_heredoc_temp_files(shell);
}

static void	handle_pipe_node(t_ast *current,
			t_env **env_list, t_shell *shell, t_ast *node)
{
	if (preprocess_all_heredocs(current, shell) != 0)
	{
		shell->exit_code = 1;
		cleanup_heredoc_temp_files(shell);
		return ;
	}
	shell->exit_code = execute_pipe(current, *env_list, shell, node);
	cleanup_heredoc_temp_files(shell);
}

void	execute_ast(t_ast *node, t_env **env_list, t_shell *shell)
{
	t_ast	*current;

	current = node;
	if (!current)
		return ;
	if (current->type == NODE_COMMAND)
		handle_command_node(current, env_list, shell);
	else if (current->type == NODE_PIPE)
		handle_pipe_node(current, env_list, shell, node);
	else if (current->type == NODE_REDIR)
		shell->exit_code = execute_redirection(current, env_list, shell);
}
