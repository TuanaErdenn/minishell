/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin3.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:38:49 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/18 14:11:29 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	expand_variable(t_env *env_list, char *str, int start)
{
	char	var_name[256];
	int		i;
	int		var_len;
	char	*value;

	i = start;
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	var_len = i - start;
	if (var_len > 0)
	{
		ft_strncpy(var_name, &str[start], var_len);
		var_name[var_len] = '\0';
		value = get_env_value(env_list, var_name);
		if (value)
			printf("%s", value);
		return (i);
	}
	printf("$");
	return (start);
}

static int	handle_dollar(t_env *env_list, char *str, int i, t_shell *shell)
{
	if (str[i + 1] == '?')
	{
		printf("%d", shell->exit_code);
		return (i + 2);
	}
	return (expand_variable(env_list, str, i + 1));
}

void	print_with_expansion(t_env *env_list, char *str, t_shell *shell)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
		{
			i = handle_dollar(env_list, str, i, shell);
		}
		else
		{
			printf("%c", str[i]);
			i++;
		}
	}
}

int	ft_echo(t_env *env_list, char **args, t_cmd cmd, t_shell *shell)
{
	int	i;
	int	newline;

	i = 1;
	newline = 1;
	while (args[i] && is_n_flag(args[i]))
	{
		newline = 0;
		i++;
	}
	(void)env_list;
	(void)shell;
	(void)cmd;
	while (args[i])
	{
		ft_putstr_fd(args[i], 1);
		if (args[i + 1])
			ft_putstr_fd(" ", 1);
		i++;
	}
	if (newline)
		ft_putstr_fd("\n", 1);
	return (0);
}

int	ft_env(t_env *env_list)
{
	t_env	*current;

	if (!env_list)
		return (0);
	current = env_list;
	while (current)
	{
		if (current->value != NULL)
		{
			ft_putstr_fd(current->key, 1);
			ft_putstr_fd("=", 1);
			ft_putstr_fd(current->value, 1);
			ft_putstr_fd("\n", 1);
		}
		current = current->next;
	}
	return (0);
}
