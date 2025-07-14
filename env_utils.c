/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 17:03:35 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/11 17:12:42 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	ft_strcmp(const char *s1, const char *s2)
{
	int	i;

	i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

t_env	*new_env_node(char *key, char *value)
{
	t_env	*new;

	new = (t_env *)malloc(sizeof(t_env));
	if (!new)
		return (NULL);
	new->key = ft_strdup(key);
	if (!new->key)
	{
		free(new);
		return (NULL);
	}
	new->value = ft_strdup(value);
	if (!new->value)
	{
		free(new->key);
		free(new);
		return (NULL);
	}
	new->next = NULL;
	return (new);
}

void	add_env_node(t_env **env_list, t_env *new_node)
{
	t_env	*temp;

	if (!*env_list)
	{
		*env_list = new_node;
		return ;
	}
	temp = *env_list;
	while (temp->next)
		temp = temp->next;
	temp->next = new_node;
}

void	split_env_line(char *line, char **key, char **value)
{
	char	*equal_sign;

	equal_sign = ft_strchr(line, '=');
	if (equal_sign)
	{
		*equal_sign = '\0';
		*key = ft_strdup(line);
		*value = ft_strdup(equal_sign + 1);
		*equal_sign = '=';
	}
	else
	{
		*key = ft_strdup(line);
		*value = ft_strdup("");
	}
}
