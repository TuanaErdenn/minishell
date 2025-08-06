/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_env_node.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 19:49:21 by terden            #+#    #+#             */
/*   Updated: 2025/07/25 19:51:12 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	allocate_key(t_env *new, char *key)
{
	new->key = ft_strdup(key);
	if (!new->key)
	{
		free(new);
		return (0);
	}
	return (1);
}

static int	allocate_value(t_env *new, char *value)
{
	if (value)
	{
		new->value = ft_strdup(value);
		if (!new->value)
		{
			free(new->key);
			free(new);
			return (0);
		}
	}
	else
		new->value = NULL;
	return (1);
}

t_env	*create_env_node(char *key, char *value)
{
	t_env	*new;

	if (!key)
		return (NULL);
	new = (t_env *)malloc(sizeof(t_env));
	if (!new)
		return (NULL);
	if (!allocate_key(new, key))
		return (NULL);
	if (!allocate_value(new, value))
		return (NULL);
	new->next = NULL;
	return (new);
}
