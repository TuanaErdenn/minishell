/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/20 20:58:07 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 19:01:18 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*create_heredoc_temp_file(t_shell *shell)
{
	char	*counter_str;
	char	*temp_file;

	counter_str = ft_itoa(shell->heredoc_counter++);
	if (!counter_str)
		return (NULL);
	temp_file = ft_strjoin("/tmp/minishell_heredoc_", counter_str);
	free(counter_str);
	return (temp_file);
}

void	cleanup_heredoc_child(t_shell *shell, t_env **env_list)
{
	rl_clear_history();
	if (shell->ast)
	{
		free_ast(shell->ast);
		shell->ast = NULL;
	}
	if (*env_list)
	{
		free_env_list(*env_list);
		*env_list = NULL;
	}
	if (shell->heredoc_temp_files)
		free_heredoc_list(shell);
}

int	setup_heredoc_child(t_shell *shell, const char *temp_file)
{
	int	fd;

	set_signal_mode(SIGMODE_HEREDOC, shell);
	free_heredoc(shell);
	fd = open(temp_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	shell->heredoc_fd = fd;
	if (fd == -1)
	{
		perror("heredoc temp file");
		exit(1);
	}
	return (fd);
}

void	write_line_to_heredoc(int fd, char *line, int should_expand,
		t_shell *shell)
{
	char	*expanded_line;

	if (should_expand)
	{
		expanded_line = expand_string_with_vars(line, shell);
		if (expanded_line)
		{
			write(fd, expanded_line, ft_strlen(expanded_line));
			free(expanded_line);
		}
		else
			write(fd, line, ft_strlen(line));
	}
	else
		write(fd, line, ft_strlen(line));
	write(fd, "\n", 1);
}

void	heredoc_input_loop(int fd, const char *delimiter, int should_expand,
		t_shell *shell)
{
	char	*line;

	while (1)
	{
		line = readline("> ");
		if (!line)
		{
			close(fd);
			printf("\nminishell: warning...\n");
			cleanup_heredoc_child(shell, &shell->env_list);
			exit(130);
		}
		if (ft_strcmp(line, delimiter) == 0)
		{
			free(line);
			break ;
		}
		write_line_to_heredoc(fd, line, should_expand, shell);
		free(line);
	}
}
