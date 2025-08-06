/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:12:43 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/08/02 18:42:26 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	only_space(char *input)
{
	int	i;

	i = 0;
	while (input[i])
	{
		if (input[i] != ' ' && input[i] != '\n' && input[i] != '\t'
			&& input[i] != '\v' && input[i] != '\f' && input[i] != '\r')
			return (0);
		i++;
	}
	return (1);
}

int	whitespace_check(char *input)
{	
	if (input[0] == '\0' || only_space(input))
	{
		if (input[0] != '\n' && input[0] != '\0')
			add_history(input);
		free(input);
		return (1);
	}
	return (0);
}

static int	handle_input(char *input, t_env *env_list, t_shell *shell)
{
	if (*input)
	{
		add_history(input);
		if (shell->needs_heredoc_cleanup && shell->heredoc_temp_files)
		{
			cleanup_heredoc_temp_files(shell);
			shell->needs_heredoc_cleanup = 0;
		}
		if (handle_exit(input, env_list))
		{
			free(input);
			return (1);
		}
		process_input(input, env_list, shell);
		if (shell->should_exit)
		{
			return (1);
		}
	}
	return (0);
}

static void	shell_loop(t_env *env_list, t_shell *shell)
{
	char	*input;

	while (1)
	{
		set_signal_mode(SIGMODE_PROMPT, shell);
		input = readline("🐣🌞 minishell ");
		if (!input)
		{
			write(1, "exit\n", 5);
			break ;
		}
		if (whitespace_check(input))
			continue ;
		check_and_reset_signal(shell);
		set_signal_mode(SIGMODE_NEUTRAL, shell);
		if (handle_input(input, env_list, shell))
			break ;
		free(input);
		input = NULL;
	}
	rl_clear_history();
}

int	main(int argc, char **argv, char **envp)
{
	t_env	*env_list;
	t_shell	shell;

	(void)argv;
	if (argc != 1)
	{
		write(1, "Usage: ./minishell\n", 19);
		return (1);
	}
	env_list = init_env_list(envp);
	if (!env_list)
	{
		ft_putstr_fd("Error: Failed to initialize environment variables\n", 2);
		return (1);
	}
	initshell(&shell, envp);
	shell.env_list = env_list;
	shell_loop(env_list, &shell);
	if (shell.heredoc_temp_files)
	{
		cleanup_heredoc_temp_files(&shell);
	}
	free_env_list(env_list);
	return (shell.exit_code);
}
