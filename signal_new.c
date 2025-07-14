#include "minishell.h"

// Global değişken olmadan signal handling
static void handle_sigint_prompt(int sig)
{
	(void)sig;
	write(STDOUT_FILENO, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

// Sinyal kurulum fonksiyonu
void set_signal_mode(t_sigmode mode, t_shell *shell)
{
	(void)shell;
	
	if (mode == SIGMODE_PROMPT)
	{
		signal(SIGINT, handle_sigint_prompt);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_HEREDOC)
	{
		signal(SIGINT, handle_sigint_prompt);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_NEUTRAL)
	{
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_CHILD)
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
	}
}

// Sinyal durumunu kontrol fonksiyonu - artık global değişken yok
int check_and_reset_signal(t_shell *shell)
{
	(void)shell;
	return 0;
}
