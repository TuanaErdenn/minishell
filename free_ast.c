/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_ast.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 20:00:20 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 19:05:49 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	free_redirections(t_ast *node)
{
	int	i;

	i = 0;
	while (i < node->redir_count)
	{
		if (node->redirections[i].filename)
			free(node->redirections[i].filename);
		if (node->redirections[i].temp_file)
			free(node->redirections[i].temp_file);
		if (node->redirections[i].delimiter)
			free(node->redirections[i].delimiter);
		if (node->redirections[i].fd != -1)
			close(node->redirections[i].fd);
		i++;
	}
	free(node->redirections);
	node->redirections = NULL;
}

static void	free_fd_and_args(t_ast *node)
{
	if (node->saved_stdin_fd != -1)
		close(node->saved_stdin_fd);
	if (node->saved_stdout_fd != -1)
		close(node->saved_stdout_fd);
	if (node->args)
	{
		free_args_array(node->args);
		node->args = NULL;
	}
	if (node->quote_types)
	{
		free(node->quote_types);
		node->quote_types = NULL;
	}
}

static void	free_file_lists(t_ast *node)
{
	free_args_array(node->input_files);
	free_args_array(node->output_files);
	free_args_array(node->append_files);
	free_args_array(node->heredoc_delims);
	free_args_array(node->heredoc_contents);
	node->input_files = NULL;
	node->output_files = NULL;
	node->append_files = NULL;
	node->heredoc_delims = NULL;
	node->heredoc_contents = NULL;
}

static void	free_quote_arrays(t_ast *node)
{
	free(node->input_quotes);
	free(node->output_quotes);
	free(node->append_quotes);
	free(node->heredoc_quotes);
	node->input_quotes = NULL;
	node->output_quotes = NULL;
	node->append_quotes = NULL;
	node->heredoc_quotes = NULL;
}

void	free_ast(t_ast *node)
{
	if (!node)
		return ;
	if (node->redirections)
		free_redirections(node);
	free_fd_and_args(node);
	free_file_lists(node);
	free_quote_arrays(node);
	if (node->file)
		free(node->file);
	if (node->left)
		free_ast(node->left);
	if (node->right)
		free_ast(node->right);
	free(node);
}
