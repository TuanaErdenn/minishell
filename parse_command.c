/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_command.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 22:42:21 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 16:38:20 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_ast	*parse_tokens(t_token **tokens)
{
	int	i;

	if (!tokens)
		return (NULL);
	if (!tokens[0])
		return (create_empty_cmd_node());
	i = 0;
	while (tokens[i])
	{
		if (tokens[i]->type == T_PIPE)
			return (parse_pipe(tokens, i));
		i++;
	}
	return (parse_command(tokens, 0, i));
}

static int	count_args_from_tokens(t_token **tokens, int start, int end)

{
	int	i;
	int	count;

	i = start;
	count = 0;
	while (i < end)
	{
		if (tokens[i]->type == T_WORD)
			count++;
		i++;
	}
	return (count);
}

static int	count_redirs(t_token **tokens, int start, int end)
{
	int	i;
	int	count;

	i = start;
	count = 0;
	while (i < end)
	{
		if (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC)
			count++;
		i++;
	}
	return (count);
}

static int	check_syntax_errors(t_token **tokens, int start, int end)
{
	int	i;

	i = start;
	while (i < end)
	{
		if (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC)
		{
			if (i + 1 >= end || (tokens[i + 1]->type >= T_INPUT
					&& tokens[i + 1]->type <= T_PIPE))
			{
				if (tokens[i]->type == T_HEREDOC)
					ft_putstr_fd("minishell:syntax error `<<'\n", 2);
				else if (tokens[i]->type == T_INPUT)
					ft_putstr_fd("minishell: syntax error `<'\n", 2);
				else if (tokens[i]->type == T_OUTPUT)
					ft_putstr_fd("minishell: syntax error  `>'\n", 2);
				else if (tokens[i]->type == T_APPEND)
					ft_putstr_fd("minishell: syntax error  `>>'\n", 2);
				return (0);
			}
		}
		i++;
	}
	return (1);
}

t_ast	*parse_command(t_token **tokens, int start, int end)
{
	t_ast	*cmd_node;
	int		arg_count;
	int		redir_count;

	if (!check_syntax_errors(tokens, start, end))
		return (NULL);
	cmd_node = create_ast_node(NODE_COMMAND);
	if (!cmd_node)
		return (NULL);
	arg_count = count_args_from_tokens(tokens, start, end);
	redir_count = count_redirs(tokens, start, end);
	if (!init_cmd_data(cmd_node, arg_count, redir_count))
	{
		free_ast(cmd_node);
		return (NULL);
	}
	if (!fill_cmd(cmd_node, tokens, start, end))
	{
		free_ast(cmd_node);
		return (NULL);
	}
	return (cmd_node);
}
