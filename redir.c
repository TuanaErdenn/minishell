/* Fixed redir.c - NO GLOBAL VARIABLES */
#include "minishell.h"
#include <sys/stat.h>

/* Helper function to setup input redirection */
static int setup_input_redirection(const char *filename, t_shell *shell)
{
    int fd;
    
    // Open the file for reading
    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror(filename);
        return -1;
    }
    
    // Save original stdin if not already saved
    if (shell->saved_stdin == -1)
    {
        shell->saved_stdin = dup(STDIN_FILENO);
        if (shell->saved_stdin == -1)
        {
            perror("dup stdin");
            close(fd);
            return -1;
        }
    }
    
    // Redirect stdin to file
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("dup2 stdin");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

/* Helper function to setup output redirection */
static int setup_output_redirection(const char *filename, t_shell *shell)
{
    int fd;
    
    // Open the file for writing (truncate)
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror(filename);
        return -1;
    }
    
    // Save original stdout if not already saved
    if (shell->saved_stdout == -1)
    {
        shell->saved_stdout = dup(STDOUT_FILENO);
        if (shell->saved_stdout == -1)
        {
            perror("dup stdout");
            close(fd);
            return -1;
        }
    }
    
    // Redirect stdout to file
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 stdout");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

/* Helper function to setup append redirection */
static int setup_append_redirection(const char *filename, t_shell *shell)
{
    int fd;
    
    // Open the file for appending
    fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        perror(filename);
        return -1;
    }
    
    // Save original stdout if not already saved
    if (shell->saved_stdout == -1)
    {
        shell->saved_stdout = dup(STDOUT_FILENO);
        if (shell->saved_stdout == -1)
        {
            perror("dup stdout");
            close(fd);
            return -1;
        }
    }
    
    // Redirect stdout to file
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 stdout");
        close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}

/* Helper function to setup heredoc */
static int setup_heredoc(const char *delimiter, t_quote_type quote_type, t_env **env_list, t_shell *shell)
{
    char *line;
    char *tmpfile;
    char *counter_str;
    char *expanded_line;
    int fd;
    int should_expand = (quote_type != Q_SINGLE); // Single quote'da expansion yok
    
    // Create temporary file - use process PID to make it unique
    pid_t pid = getpid();
    counter_str = ft_itoa(pid + shell->heredoc_counter++);
    if (!counter_str)
        return -1;
    
    tmpfile = ft_strjoin("/tmp/minishell_heredoc_", counter_str);
    free(counter_str);
    if (!tmpfile)
        return -1;
    
    // Create and write to temporary file
    fd = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1)
    {
        perror("tmpfile open");
        free(tmpfile);
        return -1;
    }
    
    // Read heredoc content
    while (1)
    {
        line = readline("> ");
        if (!line)
        {
            printf("\nminishell: warning: here-document delimited by end-of-file (wanted `%s')\n", delimiter);
            break;
        }
        
        if (ft_strcmp(line, delimiter) == 0)
        {
            free(line);
            break;
        }
        
        // Variable expansion (if not single quoted)
        if (should_expand)
        {
            expanded_line = expand_string_with_vars(line, *env_list, shell);
            if (expanded_line)
            {
                write(fd, expanded_line, ft_strlen(expanded_line));
                free(expanded_line);
            }
            else
            {
                write(fd, line, ft_strlen(line));
            }
        }
        else
        {
            write(fd, line, ft_strlen(line));
        }
        write(fd, "\n", 1);
        
        free(line);
    }
    
    close(fd);
    
    // Setup input redirection from temporary file
    int result = setup_input_redirection(tmpfile, shell);
    
    // Clean up temporary file
    unlink(tmpfile);
    free(tmpfile);
    
    return result;
}

/* ✅ SEGFAULT PREVENTION: Validate all redirections */
static int validate_all_redirections_safe(t_ast *cmd)
{
    int i;
    int test_fd;
    
    if (!cmd)
        return -1;
    
    // ✅ CRITICAL: Check input files FIRST
    i = 0;
    while (i < cmd->input_count)
    {
        // ✅ TRIPLE CHECK: array bounds and null safety
        if (!cmd->input_files || i >= cmd->input_count || 
            !cmd->input_files[i] || cmd->input_files[i][0] == '\0')
        {
            i++;
            continue;
        }
        
        if (access(cmd->input_files[i], F_OK) != 0)
        {
            perror(cmd->input_files[i]);
            return -1;  // File doesn't exist - FAIL IMMEDIATELY
        }
        if (access(cmd->input_files[i], R_OK) != 0)
        {
            perror(cmd->input_files[i]);
            return -1;  // File exists but not readable - FAIL IMMEDIATELY
        }
        i++;
    }
    
    // ✅ Check output files
    i = 0;
    while (i < cmd->output_count)
    {
        if (!cmd->output_files || i >= cmd->output_count || 
            !cmd->output_files[i] || cmd->output_files[i][0] == '\0')
        {
            i++;
            continue;
        }
        
        if (access(cmd->output_files[i], F_OK) == 0)
        {
            if (access(cmd->output_files[i], W_OK) != 0)
            {
                perror(cmd->output_files[i]);
                return -1;
            }
        }
        else
        {
            test_fd = open(cmd->output_files[i], O_WRONLY | O_CREAT | O_EXCL, 0644);
            if (test_fd == -1)
            {
                perror(cmd->output_files[i]);
                return -1;
            }
            close(test_fd);
        }
        i++;
    }
    
    // ✅ Check append files
    i = 0;
    while (i < cmd->append_count)
    {
        if (!cmd->append_files || i >= cmd->append_count || 
            !cmd->append_files[i] || cmd->append_files[i][0] == '\0')
        {
            i++;
            continue;
        }
        
        if (access(cmd->append_files[i], F_OK) == 0)
        {
            if (access(cmd->append_files[i], W_OK) != 0)
            {
                perror(cmd->append_files[i]);
                return -1;
            }
        }
        else
        {
            test_fd = open(cmd->append_files[i], O_WRONLY | O_CREAT | O_EXCL, 0644);
            if (test_fd == -1)
            {
                perror(cmd->append_files[i]);
                return -1;
            }
            close(test_fd);
        }
        i++;
    }
    
    return 0;  // All files are accessible
}

/* ✅ Main function to setup all redirections - NO GLOBALS */
int setup_redirections(t_ast *cmd, t_env **env_list, t_shell *shell)
{
    if (!cmd || !shell)
        return -1;
    
    // ✅ CRITICAL: Validate ALL files first
    if (validate_all_redirections_safe(cmd) != 0)
    {
        return -1;  // Validation failed
    }
    
    // ✅ STEP 1: Truncate all output files first (bash behavior)
    int i = 0;
    while (i < cmd->output_count)
    {
        if (!cmd->output_files || i >= cmd->output_count || 
            !cmd->output_files[i] || cmd->output_files[i][0] == '\0')
        {
            i++;
            continue;
        }
        
        int fd = open(cmd->output_files[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1)
        {
            perror(cmd->output_files[i]);
            return -1;
        }
        close(fd);
        i++;
    }
    
    // ✅ STEP 2: Handle append files
    i = 0;
    while (i < cmd->append_count)
    {
        if (!cmd->append_files || i >= cmd->append_count || 
            !cmd->append_files[i] || cmd->append_files[i][0] == '\0')
        {
            i++;
            continue;
        }
        
        // Check if already truncated by output redirection
        int already_truncated = 0;
        int j = 0;
        while (j < cmd->output_count)
        {
            if (cmd->output_files && j < cmd->output_count && cmd->output_files[j] && 
                ft_strcmp(cmd->append_files[i], cmd->output_files[j]) == 0)
            {
                already_truncated = 1;
                break;
            }
            j++;
        }
        
        if (!already_truncated)
        {
            int fd = open(cmd->append_files[i], O_WRONLY | O_CREAT, 0644);
            if (fd == -1)
            {
                perror(cmd->append_files[i]);
                return -1;
            }
            close(fd);
        }
        i++;
    }
    
    // ✅ STEP 3: Setup input redirections (last one wins)
    if (cmd->input_count > 0 && cmd->input_files)
    {
        int last_index = cmd->input_count - 1;
        if (last_index >= 0 && last_index < cmd->input_count && 
            cmd->input_files[last_index] && cmd->input_files[last_index][0] != '\0')
        {
            if (setup_input_redirection(cmd->input_files[last_index], shell) != 0)
                return -1;
        }
    }
    
    // ✅ STEP 4: Setup all heredocs (last one wins)
    i = 0;
    while (i < cmd->heredoc_count)
    {
        if (!cmd->heredoc_delims || i >= cmd->heredoc_count || 
            !cmd->heredoc_delims[i] || cmd->heredoc_delims[i][0] == '\0')
        {
            i++;
            continue;
        }
        
        t_quote_type quote_type = Q_NONE;
        if (cmd->heredoc_quotes && i < cmd->heredoc_count)
            quote_type = cmd->heredoc_quotes[i];
            
        if (setup_heredoc(cmd->heredoc_delims[i], quote_type, env_list, shell) != 0)
            return -1;
        i++;
    }
    
    // ✅ STEP 5: Redirect stdout to last output file
    if (cmd->output_count > 0 && cmd->output_files)
    {
        int last_index = cmd->output_count - 1;
        if (last_index >= 0 && last_index < cmd->output_count && 
            cmd->output_files[last_index] && cmd->output_files[last_index][0] != '\0')
        {
            if (setup_output_redirection(cmd->output_files[last_index], shell) != 0)
                return -1;
        }
    }
    // ✅ STEP 6: If no output, check for append redirections
    else if (cmd->append_count > 0 && cmd->append_files)
    {
        int last_index = cmd->append_count - 1;
        if (last_index >= 0 && last_index < cmd->append_count && 
            cmd->append_files[last_index] && cmd->append_files[last_index][0] != '\0')
        {
            if (setup_append_redirection(cmd->append_files[last_index], shell) != 0)
                return -1;
        }
    }
    
    return 0;
}

/* Function to restore original file descriptors - NO GLOBALS */
int restore_redirections(t_shell *shell)
{
    int result = 0;
    
    if (!shell)
        return -1;
    
    // Restore stdin
    if (shell->saved_stdin != -1)
    {
        if (dup2(shell->saved_stdin, STDIN_FILENO) == -1)
        {
            perror("dup2 restore stdin");
            result = -1;
        }
        close(shell->saved_stdin);
        shell->saved_stdin = -1;
    }
    
    // Restore stdout
    if (shell->saved_stdout != -1)
    {
        if (dup2(shell->saved_stdout, STDOUT_FILENO) == -1)
        {
            perror("dup2 restore stdout");
            result = -1;
        }
        close(shell->saved_stdout);
        shell->saved_stdout = -1;
    }
    
    return result;
}
/* ✅ MISSING: Add this to the end of redir.c */

/* Legacy validation function for old redirection system */
static int validate_all_redirections(t_ast *redir_node)
{
    t_ast *current = redir_node;
    int fd;

    while (current && current->type == NODE_REDIR)
    {
        if (!current->file)
        {
            current = current->left;
            continue;
        }
        
        if (current->redirect_type == REDIR_OUT)
        {
            if (access(current->file, F_OK) == 0)
            {
                if (access(current->file, W_OK) != 0)
                {
                    perror(current->file);
                    return 1;
                }
            }
            else
            {
                fd = open(current->file, O_WRONLY | O_CREAT | O_EXCL, 0644);
                if (fd == -1)
                {
                    perror(current->file);
                    return 1;
                }
                close(fd);
                unlink(current->file);
            }
        }
        else if (current->redirect_type == REDIR_APPEND)
        {
            if (access(current->file, F_OK) == 0)
            {
                if (access(current->file, W_OK) != 0)
                {
                    perror(current->file);
                    return 1;
                }
            }
            else
            {
                fd = open(current->file, O_WRONLY | O_CREAT | O_EXCL, 0644);
                if (fd == -1)
                {
                    perror(current->file);
                    return 1;
                }
                close(fd);
                unlink(current->file);
            }
        }
        else if (current->redirect_type == REDIR_IN)
        {
            if (access(current->file, F_OK | R_OK) != 0)
            {
                perror(current->file);
                return 1;
            }
        }
        current = current->left;
    }
    return 0;
}

/* ✅ MISSING: Legacy function for backward compatibility */
int execute_redirection(t_ast *redir_node, t_env **env_list, t_shell *shell)
{
    if (!redir_node || !shell)
        return 1;

    // Legacy validation
    if (validate_all_redirections(redir_node) != 0)
        return 1;  // Fail fast - don't execute command

    // Find the actual command node
    t_ast *cmd_node = redir_node;
    while (cmd_node && cmd_node->type == NODE_REDIR)
        cmd_node = cmd_node->left;
    
    if (!cmd_node)
        return 1;
    
    // Setup redirection for this single redirect (main redirect type)
    if (redir_node->redirect_type == REDIR_OUT)
    {
        if (setup_output_redirection(redir_node->file, shell) != 0)
            return 1;
    }
    else if (redir_node->redirect_type == REDIR_IN)
    {
        if (setup_input_redirection(redir_node->file, shell) != 0)
            return 1;
    }
    else if (redir_node->redirect_type == REDIR_APPEND)
    {
        if (setup_append_redirection(redir_node->file, shell) != 0)
            return 1;
    }
    else if (redir_node->redirect_type == REDIR_HEREDOC)
    {
        if (setup_heredoc(redir_node->file, redir_node->file_quote, env_list, shell) != 0)
            return 1;
    }
    
    // Execute the command
    int result = 0;
    if (cmd_node->type == NODE_PIPE)
    {
        result = execute_pipe(cmd_node, *env_list, shell, cmd_node);
    }
    else
    {
        execute_ast(cmd_node, env_list, shell);
        result = shell->exit_code;
    }
    
    // ✅ UPDATED: Restore redirections with shell parameter
    restore_redirections(shell);
    
    return result;
}