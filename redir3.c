/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir3.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 18:19:44 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 18:22:25 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	validate_output_redirection(const char *filename, int flags)
{
	int	test_fd;

	test_fd = open(filename, flags, 0644);
	if (test_fd == -1)
	{
		perror(filename);
		return (-1);
	}
	close(test_fd);
	return (0);
}

int	validate_all_redirection_files(t_ast *cmd)
{
	int	i;

	i = 0;
	while (i < cmd->redir_count)
	{
		if (cmd->redirections[i].type == T_INPUT)
		{
			if (validate_input_redirection(cmd->redirections[i].filename) != 0)
				return (-1);
		}
		else if (cmd->redirections[i].type == T_OUTPUT)
		{
			if (validate_output_redirection(cmd->redirections[i].filename,
					O_WRONLY | O_CREAT | O_TRUNC) != 0)
				return (-1);
		}
		else if (cmd->redirections[i].type == T_APPEND)
		{
			if (validate_output_redirection(cmd->redirections[i].filename,
					O_WRONLY | O_CREAT | O_APPEND) != 0)
				return (-1);
		}
		i++;
	}
	return (0);
}

int	setup_command_redirections(t_ast *cmd, t_env **env_list, t_shell *shell)
{
	int	last_input_idx;
	int	last_output_idx;

	(void)env_list;
	if (!cmd || cmd->type != NODE_COMMAND)
		return (0);
	if (cmd->saved_stdin_fd == -1)
		cmd->saved_stdin_fd = -1;
	if (cmd->saved_stdout_fd == -1)
		cmd->saved_stdout_fd = -1;
	cmd->redirections_applied = 0;
	last_input_idx = find_last_input_redirection(cmd);
	last_output_idx = find_last_output_redirection(cmd);
	if (validate_all_redirection_files(cmd) != 0)
		return (-1);
	if (last_input_idx != -1)
		if (apply_redirection(&cmd->redirections[last_input_idx], cmd,
				shell) != 0)
			return (-1);
	if (last_output_idx != -1)
		if (apply_redirection(&cmd->redirections[last_output_idx], cmd,
				shell) != 0)
			return (-1);
	cmd->redirections_applied = 1;
	return (0);
}

static void	reset_redirection_flags(t_ast *cmd)
{
	int	i;

	i = 0;
	while (i < cmd->redir_count)
	{
		cmd->redirections[i].is_applied = 0;
		i++;
	}
	cmd->redirections_applied = 0;
}

int	restore_command_redirections(t_ast *cmd)
{
	if (!cmd || !cmd->redirections_applied)
		return (0);
	if (cmd->saved_stdin_fd != -1)
	{
		if (dup2(cmd->saved_stdin_fd, STDIN_FILENO) == -1)
			perror("restore stdin");
		close(cmd->saved_stdin_fd);
		cmd->saved_stdin_fd = -1;
	}
	if (cmd->saved_stdout_fd != -1)
	{
		if (dup2(cmd->saved_stdout_fd, STDOUT_FILENO) == -1)
			perror("restore stdout");
		close(cmd->saved_stdout_fd);
		cmd->saved_stdout_fd = -1;
	}
	reset_redirection_flags(cmd);
	return (0);
}
