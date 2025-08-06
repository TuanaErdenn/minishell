/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin4.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 14:57:45 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/18 14:15:09 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	count_args(char **args)
{
	int	count;

	count = 0;
	while (args[count])
		count++;
	return (count);
}

char	*handle_home_path(t_env *env_list)
{
	char	*path;

	path = get_env_value(env_list, "HOME");
	if (!path || !*path)
	{
		ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
		return (NULL);
	}
	return (path);
}

char	*handle_oldpwd_path(t_env *env_list)
{
	char	*path;

	path = get_env_value(env_list, "OLDPWD");
	if (!path || !*path)
	{
		ft_putstr_fd("minishell: cd: OLDPWD not set\n", STDERR_FILENO);
		return (NULL);
	}
	printf("%s\n", path);
	return (path);
}

char	*handle_tilde_path(t_env *env_list, char *arg)
{
	char	*home;
	char	*path;

	home = get_env_value(env_list, "HOME");
	if (!home || !*home)
	{
		ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
		return (NULL);
	}
	path = ft_strjoin(home, &arg[1]);
	if (!path)
		return (NULL);
	return (path);
}

char	*get_cd_path(t_env *env_list, char **args)
{
	char	*path;

	if (!args[1] || ft_strcmp(args[1], "~") == 0)
		path = handle_home_path(env_list);
	else if (ft_strcmp(args[1], "-") == 0)
		path = handle_oldpwd_path(env_list);
	else if (args[1][0] == '~' && args[1][1] == '/')
		path = handle_tilde_path(env_list, args[1]);
	else
		path = args[1];
	return (path);
}
