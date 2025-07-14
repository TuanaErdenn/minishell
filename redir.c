/* FIXED redir.c - Bash-compliant redirection behavior with multiple heredoc support */
#include "minishell.h"
#include <sys/stat.h>
#include <errno.h>

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

/* ✅ FIXED: Multiple heredoc support - bash-compliant behavior with FORK */
static int setup_heredoc(const char *delimiter, t_quote_type quote_type, t_env **env_list, t_shell *shell, char **all_delimiters, int delimiter_count)
{
    (void)delimiter; // Suppress unused parameter warning
    char *line;
    char *tmpfile;
    char *counter_str;
    char *expanded_line;
    int fd;
    int should_expand = (quote_type != Q_SINGLE);
    int current_delimiter_index = 0;
    pid_t child_pid;
    int pipe_fd[2];
    int status;
    
    // Create pipe for communication
    if (pipe(pipe_fd) == -1)
    {
        perror("pipe");
        return -1;
    }
    
    // Create temporary file
    counter_str = ft_itoa(shell->heredoc_counter++);
    if (!counter_str)
    {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }
    
    tmpfile = ft_strjoin("/tmp/minishell_heredoc_", counter_str);
    free(counter_str);
    if (!tmpfile)
    {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }
    
    // Fork for heredoc processing
    child_pid = fork();
    if (child_pid == -1)
    {
        perror("fork");
        free(tmpfile);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }
    
    if (child_pid == 0)
    {
        // Child process: Handle heredoc reading
        close(pipe_fd[0]); // Close read end
        
        // Set child signal handling
        set_signal_mode(SIGMODE_CHILD, shell);
        
        // Create and write to temporary file
        fd = open(tmpfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd == -1)
        {
            perror("tmpfile open");
            free(tmpfile);
            close(pipe_fd[1]);
            cleanup_readline();
            exit(1);
        }
        
        // Read heredoc content
        while (1)
        {
            line = readline("> ");
            if (!line)
            {
                // EOF reached normally
                printf("\nminishell: warning: here-document delimited by end-of-file (wanted `%s')\n", 
                       all_delimiters[delimiter_count - 1]);
                break;
            }
            
            // Check if line matches ANY delimiter from current position
            int delimiter_found = -1;
            for (int i = current_delimiter_index; i < delimiter_count; i++)
            {
                if (ft_strcmp(line, all_delimiters[i]) == 0)
                {
                    delimiter_found = i;
                    break;
                }
            }
            
            if (delimiter_found != -1)
            {
                free(line);
                
                if (delimiter_found == delimiter_count - 1)
                {
                    // Last delimiter - stop reading
                    break;
                }
                else
                {
                    // Found intermediate delimiter - skip to next
                    current_delimiter_index = delimiter_found + 1;
                    continue;
                }
            }
            
            // Only write to file if we're on the last delimiter
            if (current_delimiter_index == delimiter_count - 1)
            {
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
            }
            
            free(line);
        }
        
        close(fd);
        close(pipe_fd[1]);
        free(tmpfile);
        
        // Clean up readline before exit to prevent memory leaks
        cleanup_readline();
        
        // Clean up environment before exit to reduce valgrind noise
        // Note: This is not strictly necessary as the OS cleans up when process exits
        // but it makes valgrind output cleaner
        exit(0);
    }
    else
    {
        // Parent process: Wait for child and handle signals
        close(pipe_fd[1]); // Close write end
        
        // Wait for child process
        waitpid(child_pid, &status, 0);
        close(pipe_fd[0]);
        
        // Check if child was interrupted by signal
        if (WIFSIGNALED(status))
        {
            shell->exit_code = 130;
            unlink(tmpfile);
            free(tmpfile);
            return -1;
        }
        
        // Setup input redirection from temporary file
        int result = setup_input_redirection(tmpfile, shell);
        
        // Clean up temporary file
        unlink(tmpfile);
        free(tmpfile);
        
        return result;
    }
}

/* ✅ FIXED: setup_redirections - Handle multiple heredocs like bash */
int setup_redirections(t_ast *cmd, t_env **env_list, t_shell *shell)
{
    if (!cmd || !shell)
        return -1;

    // ✅ STEP 1: Collect all heredoc delimiters FIRST
    char **heredoc_delimiters = NULL;
    int heredoc_count = 0;
    
    // Count heredocs
    for (int i = 0; i < cmd->redir_count; i++)
    {
        if (cmd->redirections[i].type == T_HEREDOC)
            heredoc_count++;
    }
    
    // ✅ STEP 2: Process non-heredoc redirections for validation
    for (int i = 0; i < cmd->redir_count; i++)
    {
        t_redirection *redir = &cmd->redirections[i];
        
        if (redir->type == T_OUTPUT)
        {
            // ✅ Try to create output file - FAIL IMMEDIATELY if error
            int fd = open(redir->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                perror(redir->filename);
                return -1; // ✅ STOP immediately - no further redirections
            }
            close(fd);
        }
        else if (redir->type == T_APPEND)
        {
            // ✅ Try to create/open append file - FAIL IMMEDIATELY if error
            int fd = open(redir->filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1)
            {
                perror(redir->filename);
                return -1; // ✅ STOP immediately - no further redirections
            }
            close(fd);
        }
        else if (redir->type == T_INPUT)
        {
            // ✅ Check input file - FAIL IMMEDIATELY if error
            if (access(redir->filename, F_OK) != 0)
            {
                perror(redir->filename);
                return -1; // ✅ STOP immediately - no further redirections
            }
            else if (access(redir->filename, R_OK) != 0)
            {
                perror(redir->filename);
                return -1; // ✅ STOP immediately - no further redirections
            }
        }
        // ✅ DON'T process heredocs here - we'll handle them separately
    }
    
    // ✅ STEP 3: If there are heredocs, process them ALL together
    if (heredoc_count > 0)
    {
        // Collect all heredoc delimiters
        heredoc_delimiters = malloc(sizeof(char*) * heredoc_count);
        char **original_delimiters = malloc(sizeof(char*) * heredoc_count);
        if (!heredoc_delimiters || !original_delimiters)
        {
            free(heredoc_delimiters);
            free(original_delimiters);
            return -1;
        }
        
        int idx = 0;
        for (int i = 0; i < cmd->redir_count; i++)
        {
            if (cmd->redirections[i].type == T_HEREDOC)
            {
                original_delimiters[idx] = cmd->redirections[i].filename;
                // DON'T expand heredoc delimiters - they should be literal
                heredoc_delimiters[idx] = ft_strdup(cmd->redirections[i].filename);
                if (!heredoc_delimiters[idx])
                {
                    // If strdup failed, use original and mark as such
                    heredoc_delimiters[idx] = cmd->redirections[i].filename;
                }
                idx++;
            }
        }
        
        // ✅ Process heredoc with ALL delimiters
        // Find last heredoc's quote type
        t_quote_type last_quote = Q_NONE;
        for (int i = cmd->redir_count - 1; i >= 0; i--)
        {
            if (cmd->redirections[i].type == T_HEREDOC)
            {
                last_quote = cmd->redirections[i].quote_type;
                break;
            }
        }
        
        if (setup_heredoc(heredoc_delimiters[heredoc_count-1], last_quote, 
                         env_list, shell, heredoc_delimiters, heredoc_count) != 0)
        {
            // Free expanded delimiters before returning
            for (int i = 0; i < heredoc_count; i++)
            {
                if (heredoc_delimiters[i] != original_delimiters[i])
                    free(heredoc_delimiters[i]);
            }
            free(heredoc_delimiters);
            free(original_delimiters);
            return -1;
        }
        
        // Free expanded delimiters
        for (int i = 0; i < heredoc_count; i++)
        {
            if (heredoc_delimiters[i] != original_delimiters[i])
                free(heredoc_delimiters[i]);
        }
        free(heredoc_delimiters);
        free(original_delimiters);
    }
    
    // ✅ STEP 4: Setup actual I/O redirections
    // Find last input redirection (excluding heredocs)
    int last_input = -1;
    for (int i = cmd->redir_count - 1; i >= 0; i--)
    {
        if (cmd->redirections[i].type == T_INPUT)
        {
            last_input = i;
            break;
        }
    }
    
    // ✅ CRITICAL FIX: Find last OUTPUT redirection (considering both OUTPUT and APPEND)
    int last_output_redir = -1;
    t_token_type last_output_type = T_WORD; // dummy init
    
    for (int i = cmd->redir_count - 1; i >= 0; i--)
    {
        if (cmd->redirections[i].type == T_OUTPUT || cmd->redirections[i].type == T_APPEND)
        {
            last_output_redir = i;
            last_output_type = cmd->redirections[i].type;
            break;
        }
    }
    
    // ✅ BASH BEHAVIOR: Setup stdin - heredoc overrides file input
    if (heredoc_count == 0 && last_input != -1)
    {
        // Only setup file input if there's no heredoc
        if (setup_input_redirection(cmd->redirections[last_input].filename, shell) != 0)
            return -1;
    }
    // Note: If heredoc exists, stdin is already set up by setup_heredoc
    
    // ✅ CRITICAL FIX: Setup output redirection based on LAST output/append (not type preference)
    if (last_output_redir != -1)
    {
        if (last_output_type == T_APPEND)
        {
            if (setup_append_redirection(cmd->redirections[last_output_redir].filename, shell) != 0)
                return -1;
        }
        else // T_OUTPUT
        {
            if (setup_output_redirection(cmd->redirections[last_output_redir].filename, shell) != 0)
                return -1;
        }
    }
    
    return 0;
}

/* Function to restore original file descriptors */
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

/* ✅ FIXED: Legacy function - updated for multiple heredoc support */
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
        // ✅ FIXED: Legacy heredoc with single delimiter (for backward compatibility)
        char *single_delimiter[] = {redir_node->file};
        if (setup_heredoc(redir_node->file, redir_node->file_quote, env_list, shell, single_delimiter, 1) != 0)
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
    
    // Restore redirections
    restore_redirections(shell);
    
    return result;
}