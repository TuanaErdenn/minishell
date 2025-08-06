/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 14:15:50 by terden            #+#    #+#             */
/*   Updated: 2025/08/03 14:04:51 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*handle_exit_code_expansion(char **result, t_shell *shell)
{
	char	*exit_str;
	char	*tmp;

	exit_str = NULL;
	tmp = NULL;
	exit_str = ft_itoa(shell->exit_code);
	if (!exit_str)
		return (NULL);
	tmp = ft_strjoin(*result, exit_str);
	free(*result);
	free(exit_str);
	if (!tmp)
		return (NULL);
	*result = tmp;
	return (*result);
}

char	*add_char_to_result(char **result, char c)
{
	char	temp[2];
	char	*tmp;

	tmp = NULL;
	temp[0] = c;
	temp[1] = '\0';
	tmp = ft_strjoin(*result, temp);
	free(*result);
	if (!tmp)
		return (NULL);
	*result = tmp;
	return (*result);
}

char	*handle_var_expansion(char **result, const char *str, int *i,
								t_shell *shell)
{
	int		start;
	char	*key;
	char	*val;
	char	*tmp;

	if (!result || !*result || !str || !i || !shell)
		return (NULL);
	start = *i + 1;
	while (str[start] && (ft_isalnum(str[start]) || str[start] == '_'))
		start++;
	key = ft_substr(str, *i + 1, start - (*i + 1));
	if (!key)
		return (NULL);
	val = get_env_value(shell->env_list, key);
	if (val)
		tmp = ft_strjoin(*result, val);
	else
		tmp = ft_strjoin(*result, "");
	free(*result);
	free(key);
	if (!tmp)
		return (NULL);
	*result = tmp;
	*i = start;
	return (*result);
}

char	*handle_dollar_expansion(char **result, const char *str, int *i,
								t_shell *shell)
{
	if (str[*i + 1] == '?')
	{
		if (!handle_exit_code_expansion(result, shell))
			return (NULL);
		*i += 2;
	}
	else if (str[*i + 1] >= '0' && str[*i + 1] <= '9')
	{
		*i += 2;
	}
	else if (str[*i + 1] && (ft_isalpha(str[*i + 1]) || str[*i + 1] == '_'))
	{
		if (!handle_var_expansion(result, str, i, shell))
			return (NULL);
	}
	else
	{
		if (!add_char_to_result(result, '$'))
			return (NULL);
		(*i)++;
	}
	return (*result);
}
