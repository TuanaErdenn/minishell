/* Düzeltilmiş pipe.c - Array-based redirection support */
#include "minishell.h"
#include <signal.h>

/* Pipe'ları güvenli şekilde kapatma */
static void close_pipes_safe(int *pipe_fd, int count)
{
    int i = 0;
    while (i < count)
    {
        if (pipe_fd[i] != -1)
        {
            close(pipe_fd[i]);
            pipe_fd[i] = -1;
        }
        i++;
    }
}

/* Child process cleanup - exit'ten önce belleği temizle */
static void child_cleanup_and_exit(t_env *env_list, char **envp, t_ast *original_ast, int exit_code)
{
    if (original_ast)
        free_ast(original_ast);
        
    if (env_list)
        free_env_list(env_list);
    if (envp)
        free_env_array(envp);
    exit(exit_code);
}

/* ✅ FIXED: Helper function to execute a command node in child process */
static void execute_command_in_child(t_ast *cmd_node, t_env *env_list, t_shell *child_shell, t_ast *root_ast)
{
    t_cmd cmd;
    char **envp = NULL;
    
    ft_bzero(&cmd, sizeof(t_cmd));
    cmd.args = cmd_node->args;
    
    // ✅ CRITICAL: Proper shell initialization check
    if (!child_shell)
    {
        child_cleanup_and_exit(env_list, NULL, root_ast, 1);
    }
    
    // ✅ Setup redirections for this command (NEW SYSTEM)
    if (setup_redirections(cmd_node, &env_list, child_shell) != 0)
    {
        // Redirection failed - exit with error
        child_cleanup_and_exit(env_list, NULL, root_ast, 1);
    }
    
    if (cmd_node->quote_types)
    {
        int arg_count = 0;
        while (cmd_node->args[arg_count])
            arg_count++;
        
        cmd.args_quote_type = malloc(sizeof(int) * arg_count);
        if (cmd.args_quote_type)
        {
            for (int i = 0; i < arg_count; i++)
                cmd.args_quote_type[i] = (int)cmd_node->quote_types[i];
        }
    }
    
    // Built-in mi kontrolü
    if (is_builtin(&cmd))
    {
        int result = run_builtin(env_list, &cmd, child_shell);
        if (cmd.args_quote_type)
            free(cmd.args_quote_type);
        child_cleanup_and_exit(env_list, NULL, root_ast, result);
    }
    else
    {
        // Harici komut için PATH ara
        char *path = find_exec(cmd.args[0], env_list);
        if (!path)
        {
            printf("minishell: %s: command not found\n", cmd.args[0]);
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);
            child_cleanup_and_exit(env_list, NULL, root_ast, 127);
        }
        
        // Environment array'i hazırla
        envp = env_to_array(env_list);
        if (!envp)
        {
            free(path);
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);
            child_cleanup_and_exit(env_list, NULL, root_ast, 1);
        }
        
        // Komutu çalıştır
        execve(path, cmd.args, envp);
        
        // execve başarısız olursa
        perror("execve");
        free(path);
        if (cmd.args_quote_type)
            free(cmd.args_quote_type);
        child_cleanup_and_exit(env_list, envp, root_ast, 126);
    }
}

/* ✅ FIXED: Sol child process (pipe'ın sol tarafı) */
static void execute_left_child(t_ast *left, t_env *env_list, int pipe_fd[2], t_ast *root_ast)
{
    // ✅ PROPER shell initialization - FIX for missing field initializers
    t_shell child_shell;
    child_shell.exit_code = 0;
    child_shell.envp = NULL;
    child_shell.saved_stdin = -1;
    child_shell.saved_stdout = -1;
    child_shell.heredoc_counter = 0;
    
    // STDOUT'u pipe'a yönlendir
    if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
    {
        perror("dup2 left child");
        child_cleanup_and_exit(env_list, NULL, root_ast, 1);
    }
    
    // Pipe'ları kapat
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    
    if (left->type == NODE_COMMAND)
    {
        execute_command_in_child(left, env_list, &child_shell, root_ast);
    }
    else if (left->type == NODE_REDIR)
    {
        // ✅ OLD SYSTEM: Legacy redirection node support
        t_env *env_copy = env_list;
        int result = execute_redirection(left, &env_copy, &child_shell);
        child_cleanup_and_exit(env_list, NULL, root_ast, result);
    }  
    
    child_cleanup_and_exit(env_list, NULL, root_ast, 1);
}

/* ✅ FIXED: Sağ child process (pipe'ın sağ tarafı) */
static void execute_right_child(t_ast *right, t_env *env_list, int pipe_fd[2], t_ast *root_ast)
{
    // ✅ PROPER shell initialization - FIX for missing field initializers
    t_shell child_shell;
    child_shell.exit_code = 0;
    child_shell.envp = NULL;
    child_shell.saved_stdin = -1;
    child_shell.saved_stdout = -1;
    child_shell.heredoc_counter = 0;
    
    // STDIN'i pipe'dan al
    if (dup2(pipe_fd[0], STDIN_FILENO) == -1)
    {
        perror("dup2 right child");
        child_cleanup_and_exit(env_list, NULL, root_ast, 1);
    }
    
    // Pipe'ları kapat
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    
    if (right->type == NODE_COMMAND)
    {
        execute_command_in_child(right, env_list, &child_shell, root_ast);
    }
    else if (right->type == NODE_PIPE)
    {
        // Nested pipe
        execute_pipe(right, env_list, &child_shell, root_ast);
        child_cleanup_and_exit(env_list, NULL, root_ast, child_shell.exit_code);
    }
    else if (right->type == NODE_REDIR)
    {
        // ✅ OLD SYSTEM: Legacy redirection node support
        t_env *env_copy = env_list;
        int result = execute_redirection(right, &env_copy, &child_shell);
        child_cleanup_and_exit(env_list, NULL, root_ast, result);
    }
    
    child_cleanup_and_exit(env_list, NULL, root_ast, 1);
}

/* Ana pipe execution fonksiyonu */
int execute_pipe(t_ast *pipe_node, t_env *env_list, t_shell *shell, t_ast *root_ast)
{
    int pipe_fd[2];
    pid_t left_pid, right_pid;
    int left_status, right_status;
    int final_exit_code = 0;
    
    // Pipe oluştur
    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        shell->exit_code = 1;
        return (1);
    }
    
    // Sol child process
    left_pid = fork();
    if (left_pid == -1)
    {
        perror("fork left");
        close_pipes_safe(pipe_fd, 2);
        shell->exit_code = 1;
        return (1);
    }
    
    if (left_pid == 0)
    {
        set_signal_mode(SIGMODE_CHILD, NULL);
        // Sol child - hiç return etmez, exit ile çıkar
        execute_left_child(pipe_node->left, env_list, pipe_fd, root_ast);
    }
    
    // Sağ child process
    right_pid = fork();
    if (right_pid == -1)
    {
        perror("fork right");
        close_pipes_safe(pipe_fd, 2);
        kill(left_pid, 15); // SIGTERM yerine doğrudan sayı kullan
        waitpid(left_pid, NULL, 0);
        shell->exit_code = 1;
        return (1);
    }
    
    if (right_pid == 0)
    {
        set_signal_mode(SIGMODE_CHILD, NULL);
        // Sağ child - hiç return etmez, exit ile çıkar
        execute_right_child(pipe_node->right, env_list, pipe_fd, root_ast);
    }
    
    // Parent process: pipe'ları kapat ve child'ları bekle
    close_pipes_safe(pipe_fd, 2);
    
    // Her iki child'ın da bitmesini bekle
    waitpid(left_pid, &left_status, 0);
    waitpid(right_pid, &right_status, 0);
    
    // Exit code'u sağ child'ın sonucuna göre ayarla (bash davranışı)
    if (WIFEXITED(right_status))
        final_exit_code = WEXITSTATUS(right_status);
    else if (WIFSIGNALED(right_status))
        final_exit_code = 128 + WTERMSIG(right_status);
    else
        final_exit_code = 1;
    
    shell->exit_code = final_exit_code;
    return (final_exit_code);
}