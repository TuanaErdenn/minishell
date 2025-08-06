/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 17:09:27 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/08/01 19:22:59 by terden           ###   ########.fr       */
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

static t_env	*init_empty_env(void)
{
	t_env	*env_list;
	char	*pwd;

	env_list = NULL;
	pwd = getcwd(NULL, 0);
	create_and_add_node(&env_list, ft_strdup("PWD"), pwd);
	create_and_add_node(&env_list, ft_strdup("SHLVL"), ft_strdup("1"));
	create_and_add_node(&env_list, ft_strdup("_"), ft_strdup("/usr/bin/env"));
	return (env_list);
}

t_env	*init_env_list(char **environ)
{
	t_env	*env_list;
	int		i;
	char	*key;
	char	*value;

	i = 0;
	env_list = NULL;
	if (!environ || !environ[0])
	{
		return (init_empty_env());
	}
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
