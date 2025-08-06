/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 19:52:27 by terden            #+#    #+#             */
/*   Updated: 2025/08/01 14:33:46 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	free_string_array(char **arr)
{
	int	i;

	if (!arr)
		return ;
	i = 0;
	while (arr[i])
		free(arr[i++]);
	free(arr);
}

static char	*check_direct_path(char *command)
{
	if (ft_strchr(command, '/'))
	{
		ft_putendl_fd(command, 2);
		if (access(command, F_OK) == 0 && access(command, X_OK) == 0)
			return (ft_strdup(command));
		return (NULL);
	}
	return (NULL);
}

static char	**get_path_dirs(t_env *env_list)
{
	char	*path_env;
	char	**paths;

	path_env = get_env_value(env_list, "PATH");
	if (!path_env)
		return (NULL);
	paths = ft_split(path_env, ':');
	return (paths);
}

static char	*search_in_paths(char *command, char **paths)
{
	int		i;
	char	*temp;
	char	*full_path;

	i = 0;
	while (paths[i])
	{
		temp = ft_strjoin(paths[i], "/");
		if (!temp)
			return (NULL);
		full_path = ft_strjoin(temp, command);
		free(temp);
		if (!full_path)
			return (NULL);
		if (access(full_path, F_OK) == 0 && access(full_path, X_OK) == 0)
			return (full_path);
		free(full_path);
		i++;
	}
	return (NULL);
}

char	*find_exec(char *command, t_env *env_list)
{
	char	**paths;
	char	*result;

	result = check_direct_path(command);
	if (result)
		return (result);
	paths = get_path_dirs(env_list);
	if (!paths)
		return (NULL);
	result = search_in_paths(command, paths);
	free_string_array(paths);
	return (result);
}
