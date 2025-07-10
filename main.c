/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:12:43 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/09 16:30:48 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	check_quotes(char *input)
{
	int	in_single;
	int	in_double;
	int	i;

	i = 0;
	in_single = 0;
	in_double = 0;
	while (input[i])
	{
		if (input[i] == '"' && !in_single)
			in_double = !in_double;
		else if (input[i] == '\'' && !in_double)
			in_single = !in_single;
		i++;
	}
	if (in_double || in_single)
	{
		printf("Syntax Error: Unmatched quotes\n");
		return (0);
	}
	return (1);
}

static void	process_input(char *input, t_env *env_list, t_shell *shell)
{
	t_token	**tokens;
	t_ast	*ast;

	tokens = NULL;
	ast = NULL;
	if (!input || !*input)
		return ;
	if (!check_quotes(input))
		return ;
	tokens = tokenize_with_expansion(input, env_list, shell);
	if (!tokens)
		return ;
	ast = parse_tokens(tokens);
	freetokens(tokens);
	tokens = NULL;
	if (!ast)
		return ;
	execute_ast(ast, &env_list, shell);
	if (ast)
	{
		free_ast(ast);
		ast = NULL;
	}
}

static void	initshell(t_shell *shell, char **envp)
{
	shell->exit_code = 0;
	shell->envp = envp;
	shell->saved_stdin = -1;
	shell->saved_stdout = -1;
	shell->heredoc_counter = 0;
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
		if (*input)
		{
			add_history(input);
			if (handle_exit(input, env_list))
				break ;
			process_input(input, env_list, shell);
		}
		free(input);
	}
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
	shell_loop(env_list, &shell);
	free_env_list(env_list);
	return (shell.exit_code);
}
