#include "minishell.h"

void pipe_execute(t_ast *node, t_env **env_list, t_shell *shell)
{
 int pipefd[2];
	pid_t pid1, pid2;
	int status1, status2;

	// Pipe oluştur
	if (pipe(pipefd) == -1)
	{
		perror("pipe");
		shell->exit_code = 1;
		return;
	}

	// İlk child process (sol komut - pipe'a yazacak)
	pid1 = fork();
	if (pid1 == -1)
	{
		perror("fork");
		close(pipefd[0]);
		close(pipefd[1]);
		shell->exit_code = 1;
		return;
	}

	if (pid1 == 0)
	{
		// === Child 1: Sol komut (pipe'a yazıyor) ===
		close(pipefd[0]);                         // Read ucunu kapat
		dup2(pipefd[1], STDOUT_FILENO);           // stdout → pipe write
		close(pipefd[1]);                         // Orijinal pipe write kapat
		
		execute_ast(node->left, env_list, shell); // Sol komutu çalıştır
		exit(shell->exit_code);                   // Child'ı sonlandır
	}

	// İkinci child process (sağ komut - pipe'tan okuyacak)
	pid2 = fork();
	if (pid2 == -1)
	{
		perror("fork");
		close(pipefd[0]);
		close(pipefd[1]);
		waitpid(pid1, NULL, 0); // İlk child'ı bekle
		shell->exit_code = 1;
		return;
	}

	if (pid2 == 0)
	{
		// === Child 2: Sağ komut (pipe'tan okuyor) ===
		close(pipefd[1]);                         // Write ucunu kapat
		dup2(pipefd[0], STDIN_FILENO);            // stdin → pipe read
		close(pipefd[0]);                         // Orijinal pipe read kapat
		
		execute_ast(node->right, env_list, shell); // Sağ komutu çalıştır
		exit(shell->exit_code);                    // Child'ı sonlandır
	}

	// === Parent Process ===
	close(pipefd[0]);    // Pipe uçlarını kapat
	close(pipefd[1]);
	
	// Her iki child'ı da bekle
	waitpid(pid1, &status1, 0);
	waitpid(pid2, &status2, 0);
	
	// Exit code'u son komutun sonucuna göre ayarla (bash davranışı)
	if (WIFEXITED(status2))
		shell->exit_code = WEXITSTATUS(status2);
	else
		shell->exit_code = 1;
}

