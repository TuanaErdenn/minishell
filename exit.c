/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 17:13:28 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/08/02 16:33:18 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_numeric(const char *str)
{
	int	i;

	i = 0;
	if (!str || str[0] == '\0')
		return (0);
	if (str[0] == '+' || str[0] == '-')
		i++;
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}

static long	ft_atol(const char *str)
{
	int		i;
	long	res;
	int		sign;

	i = 0;
	res = 0;
	sign = 1;
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (ft_isdigit(str[i]))
	{
		res = res * 10 + (str[i] - '0');
		i++;
	}
	return (res * sign);
}

static int	handle_non_numeric_exit(char *arg, t_shell *shell)
{
	ft_putstr_fd("minishell: exit: ", STDERR_FILENO);
	ft_putstr_fd(arg, STDERR_FILENO);
	ft_putendl_fd(": numeric argument required", STDERR_FILENO);
	shell->exit_code = 2;
	shell->should_exit = 1;
	return (2);
}

static int	handle_numeric_exit(char *arg, t_shell *shell)
{
	long	val;

	val = ft_atol(arg);
	shell->exit_code = (unsigned char)val;
	shell->should_exit = 1;
	return ((unsigned char)val);
}

int	builtin_exit(char **args, t_shell *shell)
{
	int	argc;

	argc = 0;
	while (args[argc])
		argc++;
	ft_putendl_fd("exit", STDOUT_FILENO);
	if (argc > 2)
	{
		ft_putendl_fd("minishell: exit: too many arguments", STDERR_FILENO);
		shell->exit_code = 1;
		return (1);
	}
	if (argc == 1)
	{
		shell->should_exit = 1;
		return (shell->exit_code);
	}
	if (!is_numeric(args[1]))
		return (handle_non_numeric_exit(args[1], shell));
	else
		return (handle_numeric_exit(args[1], shell));
}
