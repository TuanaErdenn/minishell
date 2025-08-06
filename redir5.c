/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir5.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 18:26:24 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 18:27:04 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	save_and_dup_stdout(t_ast *cmd, t_shell *shell, int fd)
{
	if (cmd->saved_stdout_fd == -1 && shell->saved_stdin != -2)
	{
		cmd->saved_stdout_fd = dup(STDOUT_FILENO);
		if (cmd->saved_stdout_fd == -1)
		{
			close(fd);
			perror("dup stdout");
			return (-1);
		}
	}
	if (dup2(fd, STDOUT_FILENO) == -1)
	{
		close(fd);
		perror("dup2 stdout");
		return (-1);
	}
	return (0);
}

int	validate_and_open_input_file(const char *filename)
{
	int	fd;

	if (access(filename, F_OK) != 0)
	{
		perror(filename);
		return (-1);
	}
	if (access(filename, R_OK) != 0)
	{
		perror(filename);
		return (-1);
	}
	fd = open(filename, O_RDONLY);
	if (fd == -1)
		perror(filename);
	return (fd);
}

int	apply_input_redirection(t_redirection *redir, t_ast *cmd, t_shell *shell)
{
	int	fd;

	fd = validate_and_open_input_file(redir->filename);
	if (fd == -1)
		return (-1);
	if (save_and_dup_stdin(cmd, shell, fd) != 0)
		return (-1);
	close(fd);
	return (0);
}

int	apply_output_redirection(t_redirection *redir, t_ast *cmd, t_shell *shell)
{
	int	fd;

	fd = open(redir->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
	{
		perror(redir->filename);
		return (-1);
	}
	if (save_and_dup_stdout(cmd, shell, fd) != 0)
		return (-1);
	close(fd);
	return (0);
}

int	apply_append_redirection(t_redirection *redir, t_ast *cmd, t_shell *shell)
{
	int	fd;

	fd = open(redir->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd == -1)
	{
		perror(redir->filename);
		return (-1);
	}
	if (save_and_dup_stdout(cmd, shell, fd) != 0)
		return (-1);
	close(fd);
	return (0);
}
