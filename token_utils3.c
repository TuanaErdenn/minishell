/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   token_utils3.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 17:52:45 by terden            #+#    #+#             */
/*   Updated: 2025/08/03 13:38:36 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_token	*create_token(char *value, t_token_type type)
{
	return (create_token_with_quote(value, type, Q_NONE));
}

t_token	*create_token_with_expansion(char *value, t_token_type type,
		int quote_type, int was_expanded)
{
	t_token	*token;

	token = create_token_with_quote(value, type, quote_type);
	if (token)
		token->was_expanded = was_expanded;
	return (token);
}
