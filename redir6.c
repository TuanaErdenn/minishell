/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir6.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 18:27:25 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 17:54:47 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	process_heredoc_delimiter(t_redirection *redir, t_shell *shell)
{
	if (!redir || redir->type != T_HEREDOC)
		return (0);
	set_heredoc_delimiter_and_expansion(redir);
	if (!redir->delimiter)
		return (-1);
	redir->temp_file = create_heredoc_temp_file(shell);
	if (!redir->temp_file)
	{
		free(redir->delimiter);
		redir->delimiter = NULL;
		return (-1);
	}
	if (read_heredoc(redir->delimiter, redir->should_expand,
			shell, redir->temp_file) != 0)
	{
		free(redir->delimiter);
		free(redir->temp_file);
		redir->delimiter = NULL;
		redir->temp_file = NULL;
		return (-1);
	}
	if (add_temp_file_to_list(shell, redir->temp_file) != 0)
		return (-1);
	return (0);
}

static int	process_command_heredocs(t_ast *ast, t_shell *shell)
{
	int	i;

	i = 0;
	while (i < ast->redir_count)
	{
		if (ast->redirections[i].type == T_HEREDOC)
		{
			if (process_heredoc_delimiter(&ast->redirections[i], shell) != 0)
			{
				cleanup_heredoc_temp_files(shell);
				return (-1);
			}
		}
		i++;
	}
	return (0);
}

int	preprocess_all_heredocs(t_ast *ast, t_shell *shell)
{
	if (!ast)
		return (0);
	if (ast->type == NODE_COMMAND)
		return (process_command_heredocs(ast, shell));
	if (ast->type == NODE_PIPE)
	{
		if (preprocess_all_heredocs(ast->left, shell) != 0)
			return (-1);
		if (preprocess_all_heredocs(ast->right, shell) != 0)
			return (-1);
	}
	else if (ast->type == NODE_REDIR)
	{
		if (preprocess_all_heredocs(ast->left, shell) != 0)
			return (-1);
	}
	return (0);
}

int	save_and_dup_stdin(t_ast *cmd, t_shell *shell, int fd)
{
	if (cmd->saved_stdin_fd == -1 && shell->saved_stdin != -2)
	{
		cmd->saved_stdin_fd = dup(STDIN_FILENO);
		if (cmd->saved_stdin_fd == -1)
		{
			close(fd);
			perror("dup stdin");
			return (-1);
		}
	}
	if (dup2(fd, STDIN_FILENO) == -1)
	{
		close(fd);
		perror("dup2 stdin");
		return (-1);
	}
	return (0);
}
