/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 15:28:17 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/10 19:46:20 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int remove_env(t_env **envlist, char *key)
{
	t_env	*cur;
	t_env	*prev;

	if (!envlist || !*envlist || !key)
		return (1);
	cur = *envlist;
	prev = NULL;
	if (ft_strcmp(cur->key, key) == 0)
	{
		*envlist = cur->next;
		free(cur->key);
		if (cur->value)
			free(cur->value);
		free(cur);
		return (0);
	}
	while (cur && ft_strcmp(cur->key, key) != 0)
	{
		prev = cur;
		cur = cur->next;
	}
	if (!cur)
		return (1);
	//node listeden ckar
	prev->next = cur->next;
	free(cur->key);
	if (cur->value)
		free(cur->value);
	free(cur);
	return (0);
}

int execute_unset(t_env **envlist, char **args)
{
	int	i;
	int	status;

	status = 0;
	if (!args[1])
		return (0);

	i = 1;
	while (args[i])
	{
		remove_env(envlist, args[i]);
		i++;
	}
	return (status);
}