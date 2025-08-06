/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_export_arg.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 18:48:05 by terden            #+#    #+#             */
/*   Updated: 2025/07/25 18:52:18 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	handle_no_equal(char *arg, char **key, char **value)
{
	char	*temp_key;

	temp_key = remove_quotes(arg);
	if (!temp_key)
		return (1);
	*key = temp_key;
	*value = NULL;
	return (0);
}

static int	extract_and_clean_key(char *arg, char *equal, char **key)
{
	char	*temp_key;

	*key = ft_substr(arg, 0, equal - arg);
	if (!*key)
		return (1);
	temp_key = remove_quotes(*key);
	free(*key);
	*key = temp_key;
	if (!*key)
		return (1);
	return (0);
}

static int	extract_and_clean_value(char *equal, char **value, char **key)
{
	char	*temp_value;

	temp_value = ft_strdup(equal + 1);
	if (!temp_value)
	{
		free(*key);
		*key = NULL;
		return (1);
	}
	*value = remove_quotes(temp_value);
	free(temp_value);
	if (!*value)
	{
		free(*key);
		*key = NULL;
		return (1);
	}
	return (0);
}

static int	extract_key_value(char *arg, char *equal, char **key, char **value)
{
	if (extract_and_clean_key(arg, equal, key) != 0)
		return (1);
	if (extract_and_clean_value(equal, value, key) != 0)
		return (1);
	return (0);
}

int	parse_export_arg(char *arg, char **key, char **value)
{
	char	*equal;

	if (!arg || !key || !value)
		return (1);
	equal = ft_strchr(arg, '=');
	if (!equal)
		return (handle_no_equal(arg, key, value));
	return (extract_key_value(arg, equal, key, value));
}
