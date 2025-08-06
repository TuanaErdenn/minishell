/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 15:37:52 by terden            #+#    #+#             */
/*   Updated: 2025/08/02 18:36:53 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	check_quotes(char *input)
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

void	process_input(char *input, t_env *env_list, t_shell *shell)
{
	t_token	**tokens;

	tokens = NULL;
	shell->ast = NULL;
	if (!input || !*input)
		return ;
	if (!check_quotes(input))
		return ;
	tokens = tokenize_with_expansion(input, shell);
	if (!tokens)
		return ;
	shell->ast = parse_tokens(tokens);
	freetokens(tokens);
	tokens = NULL;
	if (!shell->ast)
	{
		shell->exit_code = 2;
		return ;
	}
	execute_ast(shell->ast, &env_list, shell);
	if (shell->ast)
	{
		free_ast(shell->ast);
		shell->ast = NULL;
	}
}

void	initshell(t_shell *shell, char **envp)
{
	shell->exit_code = 0;
	shell->envp = envp;
	shell->saved_stdin = -1;
	shell->saved_stdout = -1;
	shell->heredoc_counter = 0;
	shell->interrupted = 0;
	shell->heredoc_temp_files = NULL;
	shell->heredoc_temp_count = 0;
	shell->heredoc_fd = -1;
	shell->needs_heredoc_cleanup = 0;
	shell->should_exit = 0;
}

int	handle_exit(char *input, t_env *env_list)
{
	(void)env_list;
	if (ft_strcmp(input, "exit") == 0)
	{
		write(1, "exit\n", 5);
		rl_clear_history();
		return (1);
	}
	return (0);
}
