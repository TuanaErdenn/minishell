/* Düzeltilmiş redir.c - Pipe uyumluluğu eklendi */
#include "minishell.h"

static int redir_out(t_ast *redir_node, t_env **env_list, t_shell *shell)
{
    int fd, saved_fd;
    int result = 0;
    
    // Dosyayı aç (yazma, oluştur, temizle)
    fd = open(redir_node->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open");
        return (1);
    }
    
    // stdout'u kaydet ve yönlendir
    saved_fd = dup(STDOUT_FILENO);
    if (saved_fd == -1)
    {
        perror("dup");
        close(fd);
        return (1);
    }
    
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        close(fd);
        close(saved_fd);
        return (1);
    }
    close(fd);
    
    // Alt komutu çalıştır
    if (redir_node->left)
    {
        if (redir_node->left->type == NODE_PIPE)
        {
            // ✅ FIX: Pipe içinde redirection
            result = execute_pipe(redir_node->left, *env_list, shell, redir_node->left);
        }
        else
        {
            execute_ast(redir_node->left, env_list, shell);
            result = shell->exit_code;
        }
    }
    
    // stdout'u geri yükle
    if (dup2(saved_fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 restore");
        close(saved_fd);
        return (1);
    }
    close(saved_fd);
    
    return (result);
}

static int redir_append(t_ast *redir_node, t_env **env_list, t_shell *shell)
{
    int fd, saved_fd;
    int result = 0;
    
    // Dosyayı aç (yazma, oluştur, append)
    fd = open(redir_node->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        perror("open");
        return (1);
    }
    
    // stdout'u kaydet ve yönlendir
    saved_fd = dup(STDOUT_FILENO);
    if (saved_fd == -1)
    {
        perror("dup");
        close(fd);
        return (1);
    }
    
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2");
        close(fd);
        close(saved_fd);
        return (1);
    }
    close(fd);
    
    // Alt komutu çalıştır
    if (redir_node->left)
    {
        if (redir_node->left->type == NODE_PIPE)
        {
            // ✅ FIX: Pipe içinde redirection
            result = execute_pipe(redir_node->left, *env_list, shell, redir_node->left);
        }
        else
        {
            execute_ast(redir_node->left, env_list, shell);
            result = shell->exit_code;
        }
    }
    
    // stdout'u geri yükle
    if (dup2(saved_fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 restore");
        close(saved_fd);
        return (1);
    }
    close(saved_fd);
    
    return (result);
}

static int redir_in(t_ast *redir_node, t_env **env_list, t_shell *shell)
{
    int fd, saved_fd;
    int result = 0;
    
    // Dosyayı aç (okuma)
    fd = open(redir_node->file, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return (1);
    }
    
    // stdin'i kaydet ve yönlendir
    saved_fd = dup(STDIN_FILENO);
    if (saved_fd == -1)
    {
        perror("dup");
        close(fd);
        return (1);
    }
    
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("dup2");
        close(fd);
        close(saved_fd);
        return (1);
    }
    close(fd);
    
    // Alt komutu çalıştır
    if (redir_node->left)
    {
        if (redir_node->left->type == NODE_PIPE)
        {
            // ✅ FIX: Pipe içinde redirection
            result = execute_pipe(redir_node->left, *env_list, shell, redir_node->left);
        }
        else
        {
            execute_ast(redir_node->left, env_list, shell);
            result = shell->exit_code;
        }
    }
    
    // stdin'i geri yükle
    if (dup2(saved_fd, STDIN_FILENO) == -1)
    {
        perror("dup2 restore");
        close(saved_fd);
        return (1);
    }
    close(saved_fd);
    
    return (result);
}

int execute_redirection(t_ast *redir_node, t_env **env_list, t_shell *shell)
{
    if (!redir_node)
        return (1);
        
    if (redir_node->redirect_type == REDIR_OUT) // >
        return (redir_out(redir_node, env_list, shell));
    else if (redir_node->redirect_type == REDIR_IN) // <
        return (redir_in(redir_node, env_list, shell));
    else if (redir_node->redirect_type == REDIR_APPEND) // >>
        return (redir_append(redir_node, env_list, shell));
    // TODO: REDIR_HEREDOC implementasyonu
    
    return (1);
}