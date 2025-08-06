/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 19:58:57 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 00:48:31 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	init_empty_cmd_fields(t_ast *empty_cmd)
{
	empty_cmd->input_files = NULL;
	empty_cmd->output_files = NULL;
	empty_cmd->append_files = NULL;
	empty_cmd->heredoc_delims = NULL;
	empty_cmd->heredoc_contents = NULL;
	empty_cmd->input_quotes = NULL;
	empty_cmd->output_quotes = NULL;
	empty_cmd->append_quotes = NULL;
	empty_cmd->heredoc_quotes = NULL;
	empty_cmd->input_count = 0;
	empty_cmd->output_count = 0;
	empty_cmd->append_count = 0;
	empty_cmd->heredoc_count = 0;
}

static void	init_empty_cmd_basic_fields(t_ast *empty_cmd)
{
	empty_cmd->type = NODE_COMMAND;
	empty_cmd->args[0] = NULL;
	empty_cmd->quote_types = NULL;
	empty_cmd->redirect_type = REDIR_IN;
	empty_cmd->file = NULL;
	empty_cmd->file_quote = Q_NONE;
	empty_cmd->left = NULL;
	empty_cmd->right = NULL;
	empty_cmd->redirections = NULL;
	empty_cmd->redir_count = 0;
	empty_cmd->saved_stdin_fd = -1;
	empty_cmd->saved_stdout_fd = -1;
	empty_cmd->redirections_applied = 0;
}

t_ast	*create_empty_cmd_node(void)
{
	t_ast	*empty_cmd;

	empty_cmd = malloc(sizeof(t_ast));
	if (!empty_cmd)
		return (NULL);
	empty_cmd->args = malloc(sizeof(char *));
	if (!empty_cmd->args)
	{
		free(empty_cmd);
		return (NULL);
	}
	init_empty_cmd_basic_fields(empty_cmd);
	init_empty_cmd_fields(empty_cmd);
	return (empty_cmd);
}

static t_ast	*create_pipe_node(t_ast *left, t_ast *right)
{
	t_ast	*node;

	node = create_ast_node(NODE_PIPE);
	if (!node)
	{
		free_ast(left);
		free_ast(right);
		return (NULL);
	}
	node->left = left;
	node->right = right;
	return (node);
}

t_ast	*parse_pipe(t_token **tokens, int pipe_index)
{
	t_ast	*left;
	t_ast	*right;

	if (pipe_index == 0 || !tokens[pipe_index + 1])
	{
		ft_putstr_fd("minishell: syntax error near unexpected token `|'\n", 2);
		return (NULL);
	}
	left = parse_command(tokens, 0, pipe_index);
	if (!left)
		return (NULL);
	right = parse_tokens(&tokens[pipe_index + 1]);
	if (!right)
	{
		free_ast(left);
		return (NULL);
	}
	return (create_pipe_node(left, right));
}
