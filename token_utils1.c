/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_utils1.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 17:48:00 by terden            #+#    #+#             */
/*   Updated: 2025/08/03 13:25:37 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	freetokens(t_token **tokens)
{
	int	i;

	i = 0;
	if (!tokens)
		return ;
	while (tokens[i])
	{
		if (tokens[i]->value)
			free(tokens[i]->value);
		free(tokens[i]);
		i++;
	}
	free(tokens);
}

int	is_space(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f'
		|| c == '\v');
}

int	skip_spaces(char *input, int i)
{
	while (input[i] && is_space(input[i]))
	{
		i++;
	}
	return (i);
}
