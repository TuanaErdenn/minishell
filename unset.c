/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 15:28:17 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/06/25 15:28:18 by zyilmaz          ###   ########.fr       */
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
int	is_valid_unset(char *key)
{
	int i;
	if (!key || !*key)
		return (0);
	// İlk karakter alfabetik veya '_' olmalı
	if (!ft_isalpha(key[0]) && key[0] != '_')
		return (0);
	i = 1;
	while (key[i])//digerleri alfanumerik veya_ olabilir
	{
		if (!ft_isalnum(key[i]) && key[i] != '_')
			return (0);
		i++;
	}
	return (1);
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
		if(!is_valid_unset(args[i]))
		{
			ft_putstr_fd("minishell: unset: `", 2);
			ft_putstr_fd(args[i], 2);
			ft_putstr_fd("': not a valid identifier\n", 2);
			status = 1;
		}
		else
		{
			remove_env(envlist, args[i]);
		}
		i++;
	}
	
	return (status);
}