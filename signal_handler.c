#include "minishell.h"

void handle_sigint(int sig)
{
    (void)sig;
    rl_replace_line("", 0);  // Satırı temizle
    rl_on_new_line();        // Yeni satıra geç
    rl_redisplay();          // Yeni prompt'u göster
}

void setup_signal_handlers(void)
{
    signal(SIGINT, handle_sigint);  // CTRL+C sinyali yakala
    signal(SIGQUIT, SIG_IGN);       // CTRL+\ sinyalini yok say
}
