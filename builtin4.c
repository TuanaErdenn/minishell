/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin4.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 14:57:45 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/10 19:39:48 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Argüman sayısını sayar
int	count_args(char **args)
{
	int	count;

	count = 0;
	while (args[count])
		count++;
	return (count);
}

// HOME path'ini handle eder
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

// OLDPWD path'ini handle eder
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

// ~/path formatını handle eder
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

// CD için doğru path'i belirler
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

// Dinamik olarak ayrılmış path'i temizler
void	cleanup_path(char *path, char **args)
{
	if (args[1] && args[1][0] == '~' && args[1][1] == '/')
		free(path);
}

// Dizin değiştirme işlemini yapar
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

// Argüman kontrolü yapar
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

// PWD ve OLDPWD'yi günceller
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

// Ana fonksiyon - sadece koordinasyon yapar
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
