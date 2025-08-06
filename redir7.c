/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir7.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 18:32:31 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 17:54:28 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	read_heredoc(const char *delimiter, int should_expand,
		t_shell *shell, const char *temp_file)
{
	int		fd;
	pid_t	child_pid;
	int		status;

	child_pid = fork();
	if (child_pid == -1)
	{
		perror("fork heredoc");
		return (-1);
	}
	if (child_pid == 0)
	{
		fd = setup_heredoc_child(shell, temp_file);
		heredoc_input_loop(fd, delimiter, should_expand, shell);
		close(fd);
		cleanup_heredoc_child(shell, &shell->env_list);
		exit(0);
	}
	waitpid(child_pid, &status, 0);
	return (handle_child_exit_status(status, shell, temp_file));
}

void	set_heredoc_delimiter_and_expansion(t_redirection *redir)
{
	if (redir->quote_type == Q_SINGLE)
	{
		redir->delimiter = ft_strdup(redir->filename);
		redir->should_expand = 0;
	}
	else if (redir->quote_type == Q_DOUBLE)
	{
		redir->delimiter = ft_strdup(redir->filename);
		redir->should_expand = 1;
	}
	else
	{
		redir->delimiter = ft_strdup(redir->filename);
		redir->should_expand = 1;
	}
}
