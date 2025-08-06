/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_command_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/29 18:26:37 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 17:16:20 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	init_cmd_redirections(t_ast *cmd, int redir_count)
{
	int	i;

	cmd->redirections = malloc(sizeof(t_redirection) * redir_count);
	if (!cmd->redirections)
		return (0);
	i = 0;
	while (i < redir_count)
	{
		cmd->redirections[i].type = T_WORD;
		cmd->redirections[i].filename = NULL;
		cmd->redirections[i].quote_type = Q_NONE;
		cmd->redirections[i].position = 0;
		cmd->redirections[i].fd = -1;
		cmd->redirections[i].temp_file = NULL;
		cmd->redirections[i].is_applied = 0;
		cmd->redirections[i].delimiter = NULL;
		cmd->redirections[i].should_expand = 0;
		i++;
	}
	return (1);
}

int	init_cmd_data(t_ast *cmd, int arg_count, int redir_count)
{
	if (arg_count > 0)
	{
		cmd->args = malloc(sizeof(char *) * (arg_count + 1));
		cmd->quote_types = malloc(sizeof(t_quote_type) * arg_count);
		if (!cmd->args || !cmd->quote_types)
			return (0);
	}
	else
	{
		cmd->args = malloc(sizeof(char *));
		if (!cmd->args)
			return (0);
		cmd->args[0] = NULL;
		cmd->quote_types = NULL;
	}
	if (redir_count > 0)
	{
		if (!init_cmd_redirections(cmd, redir_count))
			return (0);
	}
	else
		cmd->redirections = NULL;
	cmd->redir_count = redir_count;
	return (1);
}

static int	handle_redirection_token(t_cmd_info *info)
{
	if (info->tokens[info->i + 1] == NULL
		|| info->tokens[info->i + 1]->type != T_WORD)
	{
		if (info->tokens[info->i]->type == T_HEREDOC)
			ft_putstr_fd("minishell:syntax error `<<'\n", 2);
		else if (info->tokens[info->i]->type == T_INPUT)
			ft_putstr_fd("minishell: syntax error `<'\n", 2);
		else if (info->tokens[info->i]->type == T_OUTPUT)
			ft_putstr_fd("minishell: syntax error `>'\n", 2);
		else if (info->tokens[info->i]->type == T_APPEND)
			ft_putstr_fd("minishell: syntax error `>>'\n", 2);
		return (0);
	}
	info->cmd->redirections[info->redir_i].type = info->tokens[info->i]->type;
	info->cmd->redirections[info->redir_i].filename
		= ft_strdup(info->tokens[info->i + 1]->value);
	if (!info->cmd->redirections[info->redir_i].filename)
		return (0);
	info->cmd->redirections[info->redir_i].quote_type
		= info->tokens[info->i + 1]->quote_type;
	info->cmd->redirections[info->redir_i].position = info->pos;
	info->redir_i++;
	info->i += 2;
	info->pos += 2;
	return (1);
}

static int	handle_argument_token(t_cmd_info *info)
{
	info->cmd->args[info->arg_i] = ft_strdup(info->tokens[info->i]->value);
	if (!info->cmd->args[info->arg_i])
		return (0);
	info->cmd->quote_types[info->arg_i] = info->tokens[info->i]->quote_type;
	info->arg_i++;
	info->i++;
	info->pos++;
	return (1);
}

int	fill_cmd(t_ast *cmd, t_token **tokens, int start, int end)
{
	t_cmd_info	info;

	info = (t_cmd_info){cmd, tokens, start, 0, 0, 0};
	while (info.i < end)
	{
		if (tokens[info.i]->type >= T_INPUT
			&& tokens[info.i]->type <= T_HEREDOC)
		{
			if (!handle_redirection_token(&info))
				return (0);
		}
		else if (tokens[info.i]->type == T_WORD)
		{
			if (!handle_argument_token(&info))
				return (0);
		}
		else
		{
			info.i++;
			info.pos++;
		}
	}
	if (cmd->args)
		cmd->args[info.arg_i] = NULL;
	return (1);
}
