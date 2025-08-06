/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir8.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/30 18:33:38 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 18:38:26 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	handle_child_exit_status(int status, t_shell *shell, const char *temp_file)
{
	int	exit_code;

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
	{
		shell->interrupted = 1;
		unlink(temp_file);
		return (-1);
	}
	if (WIFEXITED(status))
	{
		exit_code = WEXITSTATUS(status);
		if (exit_code == 130)
		{
			shell->interrupted = 1;
			unlink(temp_file);
			return (-1);
		}
		if (exit_code != 0)
		{
			unlink(temp_file);
			return (-1);
		}
	}
	return (0);
}

static int	init_heredoc_list(t_shell *shell, const char *temp_file)
{
	shell->heredoc_temp_files = malloc(sizeof(char *) * 2);
	if (!shell->heredoc_temp_files)
		return (-1);
	shell->heredoc_temp_files[0] = ft_strdup(temp_file);
	shell->heredoc_temp_files[1] = NULL;
	shell->heredoc_temp_count = 1;
	return (0);
}

int	add_temp_file_to_list(t_shell *shell, const char *temp_file)
{
	char	**new_list;
	int		i;

	if (!shell->heredoc_temp_files)
		return (init_heredoc_list(shell, temp_file));
	new_list = malloc(sizeof(char *) * (shell->heredoc_temp_count + 2));
	if (!new_list)
		return (-1);
	i = 0;
	while (i < shell->heredoc_temp_count)
	{
		new_list[i] = shell->heredoc_temp_files[i];
		i++;
	}
	new_list[i] = ft_strdup(temp_file);
	new_list[i + 1] = NULL;
	free(shell->heredoc_temp_files);
	shell->heredoc_temp_files = new_list;
	shell->heredoc_temp_count++;
	return (0);
}
