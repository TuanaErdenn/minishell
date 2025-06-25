/* Düzeltilmiş pipe.c - Redirection desteği eklendi */
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

/* Redirection'ı pipe context'inde işle */
static int handle_redirection_in_pipe(t_ast *redir_node, t_env *env_list, t_shell *shell)
{
    int fd = -1;
    int saved_fd = -1;
    int result = 0;
    
    if (redir_node->redirect_type == REDIR_OUT) // >
    {
        fd = open(redir_node->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1)
        {
            perror("open");
            return (1);
        }
        saved_fd = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    else if (redir_node->redirect_type == REDIR_IN) // <
    {
        fd = open(redir_node->file, O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            return (1);
        }
        saved_fd = dup(STDIN_FILENO);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    else if (redir_node->redirect_type == REDIR_APPEND) // >>
    {
        fd = open(redir_node->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1)
        {
            perror("open");
            return (1);
        }
        saved_fd = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    // Alt komutu çalıştır (recursively handle nested redirections)
    if (redir_node->left)
    {
        if (redir_node->left->type == NODE_COMMAND)
        {
            t_cmd cmd;
            ft_bzero(&cmd, sizeof(t_cmd));
            cmd.args = redir_node->left->args;
            
            if (redir_node->left->quote_types)
            {
                int arg_count = 0;
                while (redir_node->left->args[arg_count])
                    arg_count++;
                
                cmd.args_quote_type = malloc(sizeof(int) * arg_count);
                if (cmd.args_quote_type)
                {
                    for (int i = 0; i < arg_count; i++)
                        cmd.args_quote_type[i] = (int)redir_node->left->quote_types[i];
                }
            }
            
            if (is_builtin(&cmd))
                result = run_builtin(env_list, &cmd, shell);
            else
                result = execute_command(env_list, &cmd, shell);
            
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);
        }
        else if (redir_node->left->type == NODE_REDIR)
        {
            // Nested redirection
            result = handle_redirection_in_pipe(redir_node->left, env_list, shell);
        }
    }
    
    // File descriptor'ı restore et
    if (saved_fd != -1)
    {
        if (redir_node->redirect_type == REDIR_OUT || redir_node->redirect_type == REDIR_APPEND)
            dup2(saved_fd, STDOUT_FILENO);
        else if (redir_node->redirect_type == REDIR_IN)
            dup2(saved_fd, STDIN_FILENO);
        close(saved_fd);
    }
    
    return (result);
}

/* Sol child process (pipe'ın sol tarafı) */
static void execute_left_child(t_ast *left, t_env *env_list, int pipe_fd[2], t_ast *root_ast)
{
    t_cmd cmd;
    char **envp = NULL;
    
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
        ft_bzero(&cmd, sizeof(t_cmd));
        cmd.args = left->args;
        
        if (left->quote_types)
        {
            int arg_count = 0;
            while (left->args[arg_count])
                arg_count++;
            
            cmd.args_quote_type = malloc(sizeof(int) * arg_count);
            if (cmd.args_quote_type)
            {
                for (int i = 0; i < arg_count; i++)
                    cmd.args_quote_type[i] = (int)left->quote_types[i];
            }
        }
        
        // Built-in mi kontrolü
        if (is_builtin(&cmd))
        {
            int result = run_builtin(env_list, &cmd, &(t_shell){0, NULL});
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
            perror("execve left child");
            free(path);
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);
            child_cleanup_and_exit(env_list, envp, root_ast, 126);
        }
    }
    else if (left->type == NODE_REDIR)
    {
        // ✅ FIX: Redirection'ı düzgün işle
        int result = handle_redirection_in_pipe(left, env_list, &(t_shell){0, NULL});
        child_cleanup_and_exit(env_list, NULL, root_ast, result);
    }
    
    child_cleanup_and_exit(env_list, envp, root_ast, 1);
}

/* Sağ child process (pipe'ın sağ tarafı) */
static void execute_right_child(t_ast *right, t_env *env_list, int pipe_fd[2], t_ast *root_ast)
{
    t_cmd cmd;
    char **envp = NULL;
    
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
        ft_bzero(&cmd, sizeof(t_cmd));
        cmd.args = right->args;
        
        if (right->quote_types)
        {
            int arg_count = 0;
            while (right->args[arg_count])
                arg_count++;
            
            cmd.args_quote_type = malloc(sizeof(int) * arg_count);
            if (cmd.args_quote_type)
            {
                for (int i = 0; i < arg_count; i++)
                    cmd.args_quote_type[i] = (int)right->quote_types[i];
            }
        }
        
        if (is_builtin(&cmd))
        {
            int result = run_builtin(env_list, &cmd, &(t_shell){0, NULL});
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);
            child_cleanup_and_exit(env_list, NULL, root_ast, result);
        }
        else
        {
            char *path = find_exec(cmd.args[0], env_list);
            if (!path)
            {
                printf("minishell: %s: command not found\n", cmd.args[0]);
                if (cmd.args_quote_type)
                    free(cmd.args_quote_type);
                child_cleanup_and_exit(env_list, NULL, root_ast, 127);
            }
            
            envp = env_to_array(env_list);
            if (!envp)
            {
                free(path);
                if (cmd.args_quote_type)
                    free(cmd.args_quote_type);
                child_cleanup_and_exit(env_list, NULL, root_ast, 1);
            }
            
            execve(path, cmd.args, envp);
            perror("execve right child");
            free(path);
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);
            child_cleanup_and_exit(env_list, envp, root_ast, 126);
        }
    }
    else if (right->type == NODE_PIPE)
    {
        // Nested pipe
        execute_pipe(right, env_list, &(t_shell){0, NULL}, root_ast);
        child_cleanup_and_exit(env_list, NULL, root_ast, 0);
    }
    else if (right->type == NODE_REDIR)
    {
        // ✅ FIX: Redirection'ı düzgün işle
        int result = handle_redirection_in_pipe(right, env_list, &(t_shell){0, NULL});
        child_cleanup_and_exit(env_list, NULL, root_ast, result);
    }
    
    child_cleanup_and_exit(env_list, envp, root_ast, 1);
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