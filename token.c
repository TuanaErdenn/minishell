/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/20 20:22:10 by terden            #+#    #+#             */
/*   Updated: 2025/08/03 13:34:54 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	set_token_prev_fields(t_token **tokens)
{
	int	i;

	if (!tokens)
		return ;
	i = 0;
	while (tokens[i])
	{
		if (i > 0)
			tokens[i]->prev = tokens[i - 1];
		else
			tokens[i]->prev = NULL;
		i++;
	}
}

t_token	**tokenize_with_expansion(char *input, t_shell *shell)
{
	t_token	**tokens;
	int		token_count;

	token_count = count_tokens_enhanced(input);
	tokens = (t_token **)malloc(sizeof(t_token *) * (token_count + 1));
	if (!tokens)
		return (NULL);
	if (!fill_tokens_enhanced_with_expansion(input, tokens, shell))
	{
		freetokens(tokens);
		return (NULL);
	}
	tokens[token_count] = NULL;
	set_token_prev_fields(tokens);
	return (tokens);
}
