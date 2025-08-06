/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 17:08:16 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 17:17:18 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	free_heredoc_list(t_shell *shell)
{
	int	i;

	if (!shell || !shell->heredoc_temp_files)
		return ;
	i = 0;
	while (i < shell->heredoc_temp_count)
	{
		if (shell->heredoc_temp_files[i])
		{
			free(shell->heredoc_temp_files[i]);
			shell->heredoc_temp_files[i] = NULL;
		}
		i++;
	}
	free(shell->heredoc_temp_files);
	shell->heredoc_temp_files = NULL;
	shell->heredoc_temp_count = 0;
}

void	cleanup_heredoc_temp_files(t_shell *shell)
{
	int	i;

	if (!shell || !shell->heredoc_temp_files)
		return ;
	i = 0;
	while (i < shell->heredoc_temp_count)
	{
		if (shell->heredoc_temp_files[i])
			unlink(shell->heredoc_temp_files[i]);
		i++;
	}
	free_heredoc_list(shell);
}

static int	restore_fd(int saved_fd, int std_fd, const char *label, int *result)
{
	if (dup2(saved_fd, std_fd) == -1)
	{
		perror(label);
		*result = -1;
	}
	close(saved_fd);
	return (-1);
}

int	restore_redirections(t_shell *shell)
{
	int	result;

	result = 0;
	if (!shell)
		return (-1);
	if (shell->saved_stdin != -1)
	{
		restore_fd(shell->saved_stdin, STDIN_FILENO,
			"dup2 restore stdin", &result);
		shell->saved_stdin = -1;
	}
	if (shell->saved_stdout != -1)
	{
		restore_fd(shell->saved_stdout, STDOUT_FILENO,
			"dup2 restore stdout", &result);
		shell->saved_stdout = -1;
	}
	return (result);
}

int	execute_redirection(t_ast *redir_node, t_env **env_list, t_shell *shell)
{
	t_ast	*cmd_node;
	int		result;

	result = 0;
	if (!redir_node || !shell)
		return (1);
	cmd_node = redir_node;
	while (cmd_node && cmd_node->type == NODE_REDIR)
		cmd_node = cmd_node->left;
	if (!cmd_node)
		return (1);
	execute_ast(cmd_node, env_list, shell);
	result = shell->exit_code;
	restore_redirections(shell);
	return (result);
}
