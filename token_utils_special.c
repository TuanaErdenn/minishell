/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_utils_special.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 17:52:05 by terden            #+#    #+#             */
/*   Updated: 2025/07/27 17:52:18 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	detect_pipe_or_input(char *input, int i, t_token_type *type)
{
	if (input[i] == '|')
	{
		*type = T_PIPE;
		return (i + 1);
	}
	else if (input[i] == '<')
	{
		if (input[i + 1] == '<')
		{
			*type = T_HEREDOC;
			return (i + 2);
		}
		*type = T_INPUT;
		return (i + 1);
	}
	return (i);
}

static int	detect_output(char *input, int i, t_token_type *type)
{
	if (input[i] == '>')
	{
		if (input[i + 1] == '>')
		{
			*type = T_APPEND;
			return (i + 2);
		}
		*type = T_OUTPUT;
		return (i + 1);
	}
	return (i);
}

int	create_special_token(char *input, int i, t_token **tokens,
		int *token_index)
{
	t_token_type	type;
	int				start;
	int				len;
	char			*value;

	start = i;
	type = T_WORD;
	i = detect_pipe_or_input(input, i, &type);
	i = detect_output(input, i, &type);
	len = i - start;
	value = ft_substr(input, start, len);
	if (!value)
		return (-1);
	tokens[*token_index] = create_token(value, type);
	free(value);
	if (!tokens[*token_index])
		return (-1);
	(*token_index)++;
	return (i);
}
