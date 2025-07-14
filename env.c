/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 17:09:27 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/11 17:11:30 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	create_and_add_node(t_env **env_list, char *key, char *value)
{
	t_env	*new_node;

	if (key && value)
	{
		new_node = new_env_node(key, value);
		if (new_node)
			add_env_node(env_list, new_node);
	}
	if (key)
		free(key);
	if (value)
		free(value);
}

t_env	*init_env_list(char **environ)
{
	t_env	*env_list;
	int		i;
	char	*key;
	char	*value;

	i = 0;
	env_list = NULL;
	while (environ[i])
	{
		key = NULL;
		value = NULL;
		split_env_line(environ[i], &key, &value);
		create_and_add_node(&env_list, key, value);
		i++;
	}
	return (env_list);
}

char	*get_env_value(t_env *env_list, char *key)
{
	t_env	*temp;

	if (!env_list)
		return ("");
	temp = env_list;
	while (temp)
	{
		if (ft_strcmp(temp->key, key) == 0)
			return (temp->value);
		temp = temp->next;
	}
	return ("");
}
