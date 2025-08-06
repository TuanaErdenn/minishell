/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_sorted_env.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 18:53:36 by terden            #+#    #+#             */
/*   Updated: 2025/07/25 19:33:40 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	count_env_nodes(t_env *env_list)
{
	int		count;
	t_env	*current;

	count = 0;
	current = env_list;
	while (current && ++count)
		current = current->next;
	return (count);
}

static t_env	**create_env_array(t_env *env_list, int count)
{
	t_env	**sorted_array;
	t_env	*current;
	int		i;

	sorted_array = (t_env **)malloc(sizeof(t_env *) * count);
	if (!sorted_array)
		return (NULL);
	current = env_list;
	i = -1;
	while (current)
	{
		sorted_array[++i] = current;
		current = current->next;
	}
	return (sorted_array);
}

static void	sort_env_array(t_env **sorted_array, int count)
{
	t_env	*temp;
	int		i;
	int		j;

	i = -1;
	while (++i < count - 1)
	{
		j = -1;
		while (++j < count - i - 1)
		{
			if (ft_strcmp(sorted_array[j]->key, sorted_array[j + 1]->key) > 0)
			{
				temp = sorted_array[j];
				sorted_array[j] = sorted_array[j + 1];
				sorted_array[j + 1] = temp;
			}
		}
	}
}

static void	print_env_array(t_env **sorted_array, int count)
{
	int	i;

	i = -1;
	while (++i < count)
	{
		if (!ft_strncmp(sorted_array[i]->key, "_", 2))
			continue ;
		ft_putstr_fd("declare -x ", 1);
		ft_putstr_fd(sorted_array[i]->key, 1);
		if (sorted_array[i]->value)
		{
			ft_putstr_fd("=\"", 1);
			ft_putstr_fd(sorted_array[i]->value, 1);
			ft_putstr_fd("\"", 1);
		}
		ft_putstr_fd("\n", 1);
	}
}

void	print_sorted_env(t_env **env_list)
{
	t_env	**sorted_array;
	int		count;

	if (!env_list || !*env_list)
		return ;
	count = count_env_nodes(*env_list);
	sorted_array = create_env_array(*env_list, count);
	if (!sorted_array)
		return ;
	sort_env_array(sorted_array, count);
	print_env_array(sorted_array, count);
	free(sorted_array);
}
