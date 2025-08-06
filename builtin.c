/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:41:40 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/31 14:47:35 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*ft_strncpy(char *dest, const char *src, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n && src[i])
	{
		dest[i] = src[i];
		i++;
	}
	while (i < n)
		dest[i++] = '\0';
	return (dest);
}

int	ft_pwd(t_env *env_list)
{
	char	*pwd_env;
	char	cwd[1024];

	pwd_env = get_env_value(env_list, "PWD");
	if (pwd_env && pwd_env[0] != '\0')
	{
		printf("%s\n", pwd_env);
		return (0);
	}
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("%s\n", cwd);
		return (0);
	}
	else
	{
		perror("minishell: pwd");
		return (1);
	}
}

int	is_n_flag(const char *str)
{
	int	i;

	if (!str || str[0] != '-' || str[1] != 'n')
		return (0);
	i = 2;
	while (str[i])
	{
		if (str[i] != 'n')
			return (0);
		i++;
	}
	return (1);
}

int	is_builtin(t_cmd *cmd)
{
	if (!cmd || !cmd->args || !cmd->args[0])
		return (0);
	if (!ft_strcmp(cmd->args[0], "echo"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "pwd"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "env"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "cd"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "export"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "env"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "unset"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "exit"))
		return (1);
	return (0);
}

int	run_builtin(t_env *env_list, t_cmd *cmd, t_shell *shell)
{
	if (!ft_strcmp(cmd->args[0], "echo"))
		return (ft_echo(env_list, cmd->args, *cmd, shell));
	else if (!ft_strcmp(cmd->args[0], "pwd"))
		return (ft_pwd(env_list));
	else if (!ft_strcmp(cmd->args[0], "env"))
		return (ft_env(env_list));
	else if (!ft_strcmp(cmd->args[0], "cd"))
		return (ft_cd(env_list, cmd->args));
	else if (!ft_strcmp(cmd->args[0], "export"))
		return (execute_export(&env_list, cmd->args));
	else if (!ft_strcmp(cmd->args[0], "unset"))
		return (execute_unset(&env_list, cmd->args));
	else if (!ft_strcmp(cmd->args[0], "exit"))
		return (builtin_exit(cmd->args, shell));
	return (0);
}
