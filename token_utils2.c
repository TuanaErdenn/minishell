/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_utils2.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 17:49:10 by terden            #+#    #+#             */
/*   Updated: 2025/08/03 13:36:54 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	process_special_token(char *input, int i, int *count)
{
	if (input[i] == '<' && input[i + 1] == '<')
		i += 2;
	else if (input[i] == '>' && input[i + 1] == '>')
		i += 2;
	else
		i++;
	(*count)++;
	return (i);
}

static int	process_word_token_enhanced(char *input, int i, int *count)
{
	char	quote;

	while (input[i] && !is_space(input[i]))
	{
		if (input[i] == '|' || input[i] == '<' || input[i] == '>')
			break ;
		if (input[i] == '\'' || input[i] == '\"')
		{
			quote = input[i];
			i++;
			while (input[i] && input[i] != quote)
				i++;
			if (input[i])
				i++;
		}
		else
		{
			i++;
		}
	}
	(*count)++;
	return (i);
}

int	count_tokens_enhanced(char *input)
{
	int	count;
	int	i;

	count = 0;
	i = 0;
	while (input[i])
	{
		i = skip_spaces(input, i);
		if (!input[i])
			break ;
		if (input[i] == '|' || input[i] == '<' || input[i] == '>')
			i = process_special_token(input, i, &count);
		else
			i = process_word_token_enhanced(input, i, &count);
	}
	return (count);
}

t_token	*create_token_with_quote(char *value, t_token_type type, int quote_type)
{
	t_token	*token;

	token = (t_token *)malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->value = ft_strdup(value);
	if (!token->value)
	{
		free(token);
		return (NULL);
	}
	token->type = type;
	token->quote_type = quote_type;
	token->was_expanded = 0;
	token->next = NULL;
	token->prev = NULL;
	return (token);
}
