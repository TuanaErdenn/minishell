/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/20 20:05:08 by terden            #+#    #+#             */
/*   Updated: 2025/07/30 16:59:35 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

volatile sig_atomic_t	g_interrupt_flag = 0;

void	free_heredoc(t_shell *shell)
{
	static t_shell	*temp_shell;
	t_cleanup_ctx	ctx;

	if (shell)
	{
		temp_shell = shell;
	}
	else
	{
		if (temp_shell && temp_shell->heredoc_fd != -1)
		{
			close(temp_shell->heredoc_fd);
			temp_shell->heredoc_fd = -1;
		}
		ctx.shell = temp_shell;
		ctx.env_list = temp_shell->env_list;
		ctx.envp = NULL;
		ctx.original_ast = temp_shell->ast;
		ctx.exit_code = 130;
		child_cleanup_and_exit(&ctx);
	}
}

void	set_signal_mode(t_sigmode mode, t_shell *shell)
{
	(void)shell;
	if (mode == SIGMODE_PROMPT)
	{
		signal(SIGINT, handle_sigint_parent);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_HEREDOC)
	{
		signal(SIGINT, handle_sigint_heredoc);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_NEUTRAL)
	{
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_CHILD)
	{
		signal(SIGINT, handle_sigint_child);
		signal(SIGQUIT, handle_sigquit_child);
	}
}

int	check_and_reset_signal(t_shell *shell)
{
	if (g_interrupt_flag)
	{
		g_interrupt_flag = 0;
		shell->exit_code = 130;
		return (1);
	}
	return (0);
}
