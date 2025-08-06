/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   merge_export_tokens.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 18:42:34 by terden            #+#    #+#             */
/*   Updated: 2025/07/25 18:47:42 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*merge_three_tokens(char *first_token, char **args, int i)
{
	char	*temp;
	char	*result;

	temp = ft_strjoin(first_token, "=");
	free(first_token);
	if (!temp)
		return (NULL);
	result = ft_strjoin(temp, args[i + 2]);
	free(temp);
	return (result);
}

char	*merge_export_tokens(char **args, int start_index, int *merged_count)
{
	char	*result;
	int		i;
	int		len;

	i = start_index;
	if (!args || !args[i])
		return (NULL);
	result = ft_strdup(args[i]);
	if (!result)
		return (NULL);
	*merged_count = 1;
	len = ft_strlen(result);
	if (len > 0 && result[len - 1] != '=' && args[i + 1]
		&& ft_strcmp(args[i + 1], "=") == 0 && args[i + 2])
	{
		result = merge_three_tokens(result, args, i);
		if (!result)
			return (NULL);
		*merged_count = 3;
	}
	return (result);
}
