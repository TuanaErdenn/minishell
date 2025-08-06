/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin5.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 14:14:10 by terden            #+#    #+#             */
/*   Updated: 2025/07/18 14:15:38 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	cleanup_path(char *path, char **args)
{
	if (args[1] && args[1][0] == '~' && args[1][1] == '/')
		free(path);
}

int	change_directory(char *path, char **args)
{
	if (chdir(path) != 0)
	{
		perror("minishell: cd");
		cleanup_path(path, args);
		return (1);
	}
	return (0);
}

int	validate_cd_args(char **args)
{
	int	arg_count;

	arg_count = count_args(args);
	if (arg_count > 2)
	{
		ft_putstr_fd(" too many arguments\n", STDERR_FILENO);
		return (1);
	}
	return (0);
}

int	update_pwd_env(t_env **env_list, char *current_dir, char *path, char **args)
{
	char	*new_dir;

	new_dir = getcwd(NULL, 0);
	if (!new_dir)
	{
		perror("minishell: cd");
		cleanup_path(path, args);
		return (1);
	}
	add_update_env(env_list, "OLDPWD", current_dir);
	add_update_env(env_list, "PWD", new_dir);
	free(new_dir);
	return (0);
}

int	ft_cd(t_env *env_list, char **args)
{
	char	*path;
	char	current_dir[1024];

	if (getcwd(current_dir, sizeof(current_dir)) == NULL)
	{
		perror("minishell: cd");
		return (1);
	}
	if (validate_cd_args(args) != 0)
		return (1);
	path = get_cd_path(env_list, args);
	if (!path)
		return (1);
	if (change_directory(path, args) != 0)
		return (1);
	if (update_pwd_env(&env_list, current_dir, path, args) != 0)
		return (1);
	cleanup_path(path, args);
	return (0);
}
