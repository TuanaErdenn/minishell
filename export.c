/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 19:36:34 by terden            #+#    #+#             */
/*   Updated: 2025/07/25 19:52:01 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*remove_quotes(char *str)
{
	int		len;
	char	*result;

	len = 0;
	result = NULL;
	if (!str)
		return (NULL);
	len = ft_strlen(str);
	if (len >= 2 && ((str[0] == '"' && str[len - 1] == '"')
			|| (str[0] == '\'' && str[len - 1] == '\'')))
	{
		result = ft_substr(str, 1, len - 2);
		return (result);
	}
	result = ft_strdup(str);
	return (result);
}

int	add_update_env(t_env **env_list, char *key, char *value)
{
	t_env	*cur;
	t_env	*new_node;

	if (!env_list || !key)
		return (1);
	cur = *env_list;
	while (cur)
	{
		if (ft_strcmp(cur->key, key) == 0)
		{
			if (cur->value)
				free(cur->value);
			if (value)
				cur->value = ft_strdup(value);
			else
				cur->value = NULL;
			return (0);
		}
		cur = cur->next;
	}
	new_node = create_env_node(key, value);
	if (!new_node)
		return (1);
	add_env_node(env_list, new_node);
	return (0);
}

int	is_valid_identifier(char *key)
{
	int	i;

	if (!key || !*key)
		return (0);
	if (!ft_isalpha(key[0]) && key[0] != '_')
		return (0);
	i = 1;
	while (key[i])
	{
		if (!ft_isalnum(key[i]) && key[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int	execute_export(t_env **env_list, char **args)
{
	int	i;
	int	status;
	int	tokens_used;

	status = 0;
	if (!args[1])
	{
		print_sorted_env(env_list);
		return (0);
	}
	i = 1;
	while (args[i])
	{
		tokens_used = process_export_arg(env_list, args, i, &status);
		if (tokens_used <= 0)
			i++;
		else
			i += tokens_used;
	}
	return (status);
}
