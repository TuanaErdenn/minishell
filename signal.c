#include "minishell.h"

static t_shell	*shell_ref = NULL;

// Ctrl+C (Prompt)
static void	handle_sigint_prompt(int sig)
{
	(void)sig;
	write(STDOUT_FILENO, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
	if (shell_ref)
		shell_ref->exit_code = 130;
}

// Ctrl+C (Heredoc)
static void	handle_sigint_heredoc(int sig)
{
	(void)sig;
	write(STDOUT_FILENO, "\n", 1);
	close(STDIN_FILENO);
	if (shell_ref)
		shell_ref->exit_code = 130;
}

// Genel sinyal kurulum fonksiyonu
void	set_signal_mode(t_sigmode mode, t_shell *shell)
{
	shell_ref = shell;
	if (mode == SIGMODE_PROMPT)
	{
		signal(SIGINT, handle_sigint_prompt);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_HEREDOC)
	{
		signal(SIGINT, handle_sigint_heredoc);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_CHILD)
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
	}
}
