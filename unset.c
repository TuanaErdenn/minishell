/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 15:28:17 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/25 20:08:48 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	free_env_node(t_env *node)
{
	if (!node)
		return ;
	if (node->key)
		free(node->key);
	if (node->value)
		free(node->value);
	free(node);
}

static int	remove_head_node(t_env **envlist, t_env *head)
{
	*envlist = head->next;
	free_env_node(head);
	return (0);
}

int	remove_env(t_env **envlist, char *key)
{
	t_env	*cur;
	t_env	*prev;

	if (!envlist || !*envlist || !key)
		return (1);
	cur = *envlist;
	prev = NULL;
	if (cur && cur->key && ft_strcmp(cur->key, key) == 0)
		return (remove_head_node(envlist, cur));
	while (cur && cur->key && ft_strcmp(cur->key, key) != 0)
	{
		prev = cur;
		cur = cur->next;
	}
	if (!cur || !cur->key)
		return (1);
	prev->next = cur->next;
	free_env_node(cur);
	return (0);
}

int	execute_unset(t_env **envlist, char **args)
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
