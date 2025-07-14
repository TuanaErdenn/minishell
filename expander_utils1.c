/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils1.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 18:55:21 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/11 19:06:30 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int should_remove_arg(const char *original, const char *expanded, t_quote_type quote_type)
{
	// If argument was quoted, never remove it (even if it becomes empty)
	if (quote_type != Q_NONE)
		return 0;

	// If original contained only a variable reference that expanded to empty, remove it
	if (original && original[0] == '$' && expanded && expanded[0] == '\0')
	{
		// Check if it's a pure variable reference (no other characters)
		int i = 1;
		while (original[i] && (ft_isalnum(original[i]) || original[i] == '_'))
			i++;

		// If we reached the end, it was a pure variable reference
		if (original[i] == '\0')
			return 1;
	}

	return 0;
}
void	free_str_array(char **arr)
{
	int	i;

	i = 0;
	if (!arr)
		return;
	while (arr[i])
		free(arr[i++]);
	free(arr);
}

