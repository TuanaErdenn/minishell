/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_ast_node.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 20:08:04 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 13:05:33 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	init_ast_io_fields(t_ast *node)
{
	node->input_files = NULL;
	node->output_files = NULL;
	node->append_files = NULL;
	node->heredoc_delims = NULL;
	node->heredoc_contents = NULL;
	node->input_quotes = NULL;
	node->output_quotes = NULL;
	node->append_quotes = NULL;
	node->heredoc_quotes = NULL;
	node->input_count = 0;
	node->output_count = 0;
	node->append_count = 0;
	node->heredoc_count = 0;
}

t_ast	*create_ast_node(t_node_type type)
{
	t_ast	*node;

	node = malloc(sizeof(t_ast));
	if (!node)
		return (NULL);
	node->type = type;
	node->args = NULL;
	node->quote_types = NULL;
	node->redirect_type = 0;
	node->file = NULL;
	node->file_quote = Q_NONE;
	node->left = NULL;
	node->right = NULL;
	node->redirections = NULL;
	node->redir_count = 0;
	node->saved_stdin_fd = -1;
	node->saved_stdout_fd = -1;
	node->redirections_applied = 0;
	init_ast_io_fields(node);
	return (node);
}
