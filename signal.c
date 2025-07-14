#include "minishell.h"

// Global interrupt flag (sadece bu dosya için)
static volatile sig_atomic_t g_interrupt_flag = 0;

// Parent process için SIGINT handler - prompt'ta newline + redisplay
static void handle_sigint_parent(int sig)
{
	(void)sig;
	g_interrupt_flag = 1;
	write(STDOUT_FILENO, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

// Child process için SIGINT handler - sadece exit kodu ayarla
static void handle_sigint_child(int sig)
{
	(void)sig;
	exit(130);
}

// Sinyal kurulum fonksiyonu
void set_signal_mode(t_sigmode mode, t_shell *shell)
{
	(void)shell;
	
	if (mode == SIGMODE_PROMPT)
	{
		signal(SIGINT, handle_sigint_parent);
		signal(SIGQUIT, SIG_IGN);
	}
	else if (mode == SIGMODE_HEREDOC)
	{
		signal(SIGINT, handle_sigint_parent);
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
		signal(SIGQUIT, SIG_DFL);
	}
}

// Sinyal durumunu kontrol fonksiyonu
int check_and_reset_signal(t_shell *shell)
{
	if (g_interrupt_flag)
	{
		g_interrupt_flag = 0;
		shell->exit_code = 130;
		return (1);
	}
	return (0);
}
