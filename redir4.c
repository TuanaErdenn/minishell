/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir4.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 18:24:47 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 18:25:45 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	apply_heredoc_redirection(t_redirection *redir, t_ast *cmd, t_shell *shell)
{
	int	fd;

	if (!redir->temp_file)
	{
		printf("Error: Heredoc not preprocessed\n");
		return (-1);
	}
	fd = open(redir->temp_file, O_RDONLY);
	if (fd == -1)
	{
		perror("heredoc temp file");
		return (-1);
	}
	redir->fd = fd;
	if (save_and_dup_stdin(cmd, shell, fd) != 0)
		return (-1);
	close(fd);
	return (0);
}

int	apply_redirection(t_redirection *redir, t_ast *cmd, t_shell *shell)
{
	if (!redir || redir->is_applied)
		return (0);
	if (redir->type == T_INPUT
		&& apply_input_redirection(redir, cmd, shell) != 0)
		return (-1);
	if (redir->type == T_OUTPUT
		&& apply_output_redirection(redir, cmd, shell) != 0)
		return (-1);
	if (redir->type == T_APPEND
		&& apply_append_redirection(redir, cmd, shell) != 0)
		return (-1);
	if (redir->type == T_HEREDOC
		&& apply_heredoc_redirection(redir, cmd, shell) != 0)
		return (-1);
	if (redir->type != T_INPUT && redir->type != T_OUTPUT
		&& redir->type != T_APPEND && redir->type != T_HEREDOC)
		return (0);
	redir->is_applied = 1;
	return (0);
}

int	find_last_input_redirection(t_ast *cmd)
{
	int	i;

	i = cmd->redir_count - 1;
	while (i >= 0)
	{
		if (cmd->redirections[i].type == T_INPUT
			|| cmd->redirections[i].type == T_HEREDOC)
			return (i);
		i--;
	}
	return (-1);
}

int	find_last_output_redirection(t_ast *cmd)
{
	int	i;

	i = cmd->redir_count - 1;
	while (i >= 0)
	{
		if (cmd->redirections[i].type == T_OUTPUT
			|| cmd->redirections[i].type == T_APPEND)
			return (i);
		i--;
	}
	return (-1);
}

int	validate_input_redirection(const char *filename)
{
	if (access(filename, F_OK) != 0)
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd((char *)filename, 2);
		ft_putstr_fd(": No such file or directory\n", 2);
		return (-1);
	}
	if (access(filename, R_OK) != 0)
	{
		perror(filename);
		return (-1);
	}
	return (0);
}
