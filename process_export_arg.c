/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process_export_arg.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/25 19:34:13 by terden            #+#    #+#             */
/*   Updated: 2025/07/25 19:47:49 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	handle_parse_error(char *merged_arg, int merged_count, int *status)
{
	ft_putstr_fd("minishell: export: error parsing argument\n", 2);
	*status = 1;
	free(merged_arg);
	return (merged_count);
}

static void	handle_invalid_identifier(char *merged_arg, int *status)
{
	ft_putstr_fd("minishell: export: `", 2);
	ft_putstr_fd(merged_arg, 2);
	ft_putstr_fd("': not a valid identifier\n", 2);
	*status = 1;
}

static void	cleanup_export_data(char *key, char *value, char *merged_arg)
{
	free_key_value(key, value);
	free(merged_arg);
}

int	process_export_arg(t_env **env_list, char **args, int i, int *status)
{
	char	*key;
	char	*value;
	char	*merged_arg;
	int		merged_count;

	key = NULL;
	value = NULL;
	merged_count = 0;
	merged_arg = merge_export_tokens(args, i, &merged_count);
	if (!merged_arg)
	{
		ft_putstr_fd("minishell: export: error merging tokens\n", 2);
		*status = 1;
		return (1);
	}
	if (parse_export_arg(merged_arg, &key, &value) != 0)
		return (handle_parse_error(merged_arg, merged_count, status));
	if (!is_valid_identifier(key))
		handle_invalid_identifier(merged_arg, status);
	else if (add_update_env(env_list, key, value) != 0)
		*status = 1;
	cleanup_export_data(key, value, merged_arg);
	return (merged_count);
}
