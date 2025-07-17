/* ✅ IMPROVED parse.c - Enhanced AST with proper fd management */
#include "minishell.h"

/* Helper function to safely free args array */
static void free_args_array(char **args)
{
	if (!args)
		return;

	int i = 0;
	while (args[i])
	{
		free(args[i]);
		args[i] = NULL;
		i++;
	}
	free(args);
}

/* ✅ UPDATED: Enhanced free_ast with new redirection cleanup */
void free_ast(t_ast *node)
{
    if (!node)
        return;

    /* ✅ Clean up NEW redirection array with enhanced fields */
    if (node->redirections)
    {
        for (int i = 0; i < node->redir_count; i++)
        {
            if (node->redirections[i].filename)
                free(node->redirections[i].filename);
            if (node->redirections[i].temp_file)
                free(node->redirections[i].temp_file);
            if (node->redirections[i].delimiter)
                free(node->redirections[i].delimiter);
            
            /* Close any open file descriptors */
            if (node->redirections[i].fd != -1)
                close(node->redirections[i].fd);
        }
        free(node->redirections);
        node->redirections = NULL;
    }

    /* ✅ Close backup file descriptors */
    if (node->saved_stdin_fd != -1)
        close(node->saved_stdin_fd);
    if (node->saved_stdout_fd != -1)
        close(node->saved_stdout_fd);

    /* ✅ Clean up existing fields */
    if (node->args)
    {
        free_args_array(node->args);
        node->args = NULL;
    }

    if (node->quote_types)
    {
        free(node->quote_types);
        node->quote_types = NULL;
    }

    /* Clean up legacy redirection arrays */
    if (node->input_files)
    {
        free_args_array(node->input_files);
        node->input_files = NULL;
    }
    if (node->output_files)
    {
        free_args_array(node->output_files);
        node->output_files = NULL;
    }
    if (node->append_files)
    {
        free_args_array(node->append_files);
        node->append_files = NULL;
    }
    if (node->heredoc_delims)
    {
        free_args_array(node->heredoc_delims);
        node->heredoc_delims = NULL;
    }
    if (node->heredoc_contents)
    {
        free_args_array(node->heredoc_contents);
        node->heredoc_contents = NULL;
    }
    
    /* Clean up quote arrays */
    if (node->input_quotes)
    {
        free(node->input_quotes);
        node->input_quotes = NULL;
    }
    if (node->output_quotes)
    {
        free(node->output_quotes);
        node->output_quotes = NULL;
    }
    if (node->append_quotes)
    {
        free(node->append_quotes);
        node->append_quotes = NULL;
    }
    if (node->heredoc_quotes)
    {
        free(node->heredoc_quotes);
        node->heredoc_quotes = NULL;
    }

    if (node->file)
    {
        free(node->file);
        node->file = NULL;
    }

    /* Clean up child nodes recursively */
    if (node->left)
    {
        free_ast(node->left);
        node->left = NULL;
    }

    if (node->right)
    {
        free_ast(node->right);
        node->right = NULL;
    }

    /* Free the node itself */
    free(node);
}

/* ✅ HELPER: Initialize AST node with all fields */
static t_ast *create_ast_node(t_node_type type)
{
    t_ast *node = malloc(sizeof(t_ast));
    if (!node)
        return NULL;
        
    /* Initialize all fields to safe values */
    node->type = type;
    node->args = NULL;
    node->quote_types = NULL;
    node->redirect_type = 0;
    node->file = NULL;
    node->file_quote = Q_NONE;
    node->left = NULL;
    node->right = NULL;
    
    /* ✅ NEW: Initialize enhanced redirection fields */
    node->redirections = NULL;
    node->redir_count = 0;
    node->saved_stdin_fd = -1;
    node->saved_stdout_fd = -1;
    node->redirections_applied = 0;
    
    /* Initialize legacy redirection arrays */
    node->input_files = NULL;
    node->output_files = NULL;
    node->append_files = NULL;
    node->heredoc_delims = NULL;
    node->heredoc_contents = NULL;
    node->input_quotes = NULL;
    node->output_quotes = NULL;
    node->append_quotes = NULL;
    node->heredoc_quotes = NULL;
    node->input_count = 0;
    node->output_count = 0;
    node->append_count = 0;
    node->heredoc_count = 0;

    return node;
}

/* ADDITIONAL: Güvenli parse_tokens wrapper */
t_ast *parse_tokens(t_token **tokens)
{
	int i = 0;

	if (!tokens || !tokens[0])
		return NULL;

	/* Token dizisinde PIPE arıyoruz */
	while (tokens[i])
	{
		if (tokens[i]->type == T_PIPE)
			return parse_pipe(tokens, i); /* İlk PIPE bulunduğunda parçala */
		i++;
	}

	/* PIPE yoksa tüm aralık komut olarak gönder */
	return parse_command(tokens, 0, i);
}

t_ast *parse_pipe(t_token **tokens, int pipe_index)
{
	t_ast *left = NULL;
	t_ast *right = NULL;
	t_ast *node = NULL;

	/* Pipe syntax kontrolü */
	if (pipe_index == 0)
	{
		/* Pipe'dan önce hiçbir şey yok */
		ft_putstr_fd("minishell: syntax error near unexpected token `|'\n", 2);
		return NULL;
	}
	
	if (!tokens[pipe_index + 1])
	{
		/* Pipe'dan sonra hiçbir şey yok */
		ft_putstr_fd("minishell: syntax error near unexpected token `|'\n", 2);
		return NULL;
	}

	/* Sol kısmı parse et */
	left = parse_command(tokens, 0, pipe_index);
	if (!left)
		return NULL;

	/* Sağ kısmı parse et (pipe'dan sonraki tokenlar) */
	right = parse_tokens(&tokens[pipe_index + 1]);
	if (!right)
	{
		free_ast(left); /* Sol kısmı temizle */
		return NULL;
	}

	/* ✅ UPDATED: Use helper function for consistent initialization */
	node = create_ast_node(NODE_PIPE);
	if (!node)
	{
		free_ast(left); /* Her iki kısmı da temizle */
		free_ast(right);
		return NULL;
	}

	node->left = left;
	node->right = right;

	return node;
}

/* Helper function to add redirection to arrays */
int add_redirection(char ***files, t_quote_type **quotes, int *count, 
                    const char *filename, t_quote_type quote_type)
{
	char **new_files;
	t_quote_type *new_quotes;

	/* +2: biri yeni eleman, biri NULL sonlandırma için */
	new_files = realloc(*files, sizeof(char *) * (*count + 2));
	new_quotes = realloc(*quotes, sizeof(t_quote_type) * (*count + 1));

	if (!new_files || !new_quotes)
	{
		free(new_files);
		free(new_quotes);
		return 0;
	}

	*files = new_files;
	*quotes = new_quotes;

	(*files)[*count] = ft_strdup(filename);
	if (!(*files)[*count])
		return 0;

	(*quotes)[*count] = quote_type;
	(*count)++;

	(*files)[*count] = NULL;  /* ✅ NULL SONLANDIRMA */
	return 1;
}

/* ✅ UPDATED: Enhanced parse_command with better redirection handling */
t_ast *parse_command(t_token **tokens, int start, int end)
{
    t_ast *cmd_node = NULL;
    char **args = NULL;
    t_quote_type *quotes = NULL;
    t_redirection *redirections = NULL;
    int i = start;
    int arg_count = 0;
    int redir_count = 0;

    /* ✅ UPDATED: Use helper function for consistent initialization */
    cmd_node = create_ast_node(NODE_COMMAND);
    if (!cmd_node)
        return NULL;

    /* ✅ PASS 1: Count arguments and redirections */
    i = start;
    while (i < end)
    {
        if (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC)
        {
            redir_count++;
            i += 2; /* Skip operator and filename */
            if (i > end)
            {
                printf("Syntax Error: Missing file after redirection\n");
                free_ast(cmd_node);
                return NULL;
            }
        }
        else if (tokens[i]->type == T_WORD)
        {
            arg_count++;
            i++;
        }
        else
        {
            i++;
        }
    }

    /* ✅ PASS 2: Allocate arrays */
    if (arg_count > 0)
    {
        args = malloc(sizeof(char *) * (arg_count + 1));
        quotes = malloc(sizeof(t_quote_type) * arg_count);
        if (!args || !quotes)
        {
            free(args);
            free(quotes);
            free_ast(cmd_node);
            return NULL;
        }
    }

    if (redir_count > 0)
    {
        redirections = malloc(sizeof(t_redirection) * redir_count);
        if (!redirections)
        {
            free(args);
            free(quotes);
            free_ast(cmd_node);
            return NULL;
        }
        
        /* ✅ CRITICAL: Initialize all redirection fields */
        for (int j = 0; j < redir_count; j++)
        {
            redirections[j].type = T_WORD; /* Safe default */
            redirections[j].filename = NULL;
            redirections[j].quote_type = Q_NONE;
            redirections[j].position = 0;
            redirections[j].fd = -1;
            redirections[j].temp_file = NULL;
            redirections[j].is_applied = 0;
            redirections[j].delimiter = NULL;
            redirections[j].should_expand = 0;
        }
    }

    /* ✅ PASS 3: Fill arrays in ORDER */
    int arg_index = 0;
    int redir_index = 0;
    int position = 0;  /* Track position in command line */
    
    i = start;
    while (i < end)
    {
        if (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC)
        {
            /* Handle redirection - PRESERVE ORDER */
            if (i + 1 >= end || tokens[i + 1]->type != T_WORD)
            {
                printf("Syntax Error: Missing file after redirection\n");
                /* Cleanup */
                while (--arg_index >= 0)
                    free(args[arg_index]);
                free(args);
                free(quotes);
                for (int j = 0; j < redir_index; j++)
                {
                    if (redirections[j].filename)
                        free(redirections[j].filename);
                }
                free(redirections);
                free_ast(cmd_node);
                return NULL;
            }

            /* ✅ CRITICAL: Store redirection in ORDER with enhanced fields */
            redirections[redir_index].type = tokens[i]->type;
            redirections[redir_index].filename = ft_strdup(tokens[i + 1]->value);
            redirections[redir_index].quote_type = tokens[i + 1]->quote_type;
            redirections[redir_index].position = position;
            
            if (!redirections[redir_index].filename)
            {
                /* Cleanup on failure */
                while (--arg_index >= 0)
                    free(args[arg_index]);
                free(args);
                free(quotes);
                for (int j = 0; j < redir_index; j++)
                {
                    if (redirections[j].filename)
                        free(redirections[j].filename);
                }
                free(redirections);
                free_ast(cmd_node);
                return NULL;
            }
            
            redir_index++;
            i += 2; /* Skip operator and filename */
            position += 2;
        }
        else if (tokens[i]->type == T_WORD)
        {
            /* Handle command argument */
            args[arg_index] = ft_strdup(tokens[i]->value);
            if (!args[arg_index])
            {
                /* Cleanup on failure */
                while (--arg_index >= 0)
                    free(args[arg_index]);
                free(args);
                free(quotes);
                for (int j = 0; j < redir_index; j++)
                {
                    if (redirections[j].filename)
                        free(redirections[j].filename);
                }
                free(redirections);
                free_ast(cmd_node);
                return NULL;
            }
            quotes[arg_index] = tokens[i]->quote_type;
            arg_index++;
            i++;
            position++;
        }
        else
        {
            i++;
            position++;
        }
    }
    
    /* ✅ Finalize arrays */
    if (args)
    {
        args[arg_count] = NULL;
        cmd_node->args = args;
        cmd_node->quote_types = quotes;
    }
    else
    {
        cmd_node->args = malloc(sizeof(char *) * 1);
        if (!cmd_node->args)
        {
            for (int j = 0; j < redir_index; j++)
            {
                if (redirections[j].filename)
                    free(redirections[j].filename);
            }
            free(redirections);
            free_ast(cmd_node);
            return NULL;
        }
        cmd_node->args[0] = NULL;
        cmd_node->quote_types = NULL;
    }

    cmd_node->redirections = redirections;
    cmd_node->redir_count = redir_count;

    return cmd_node;
}

/* ✅ UPDATED: Enhanced execute_ast with proper heredoc timing */
void execute_ast(t_ast *node, t_env **env_list, t_shell *shell)
{
    t_cmd cmd;
    t_ast *current = node;

    if (!current)
        return;

    if (current->type == NODE_COMMAND)
    {
        /* ✅ STEP 1: Pre-process heredocs FIRST */
        if (preprocess_all_heredocs(current, env_list, shell) != 0)
        {
            shell->exit_code = 1;
            cleanup_heredoc_temp_files(shell);
            return;
        }

        /* t_cmd yapısını hazırla */
        ft_bzero(&cmd, sizeof(t_cmd));
        cmd.args = current->args;

        /* Quote_types'ları kopyala */
        if (current->quote_types && current->args)
        {
            int arg_count = 0;
            while (current->args[arg_count])
                arg_count++;

            cmd.args_quote_type = malloc(sizeof(int) * arg_count);
            if (cmd.args_quote_type)
            {
                for (int i = 0; i < arg_count; i++)
                    cmd.args_quote_type[i] = (int)current->quote_types[i];
            }
        }
        else
        {
            cmd.args_quote_type = NULL;
        }

        /* ✅ STEP 2: Setup redirections using NEW system */
        int redir_result = setup_command_redirections(current, env_list, shell);
        if (redir_result != 0)
        {
            shell->exit_code = 1;
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);
            cleanup_heredoc_temp_files(shell);
            return;
        }

        /* ✅ STEP 3: Execute command */
        shell->exit_code = execute_command_common(*env_list, &cmd, shell, 0);

        /* ✅ STEP 4: Restore redirections */
        restore_command_redirections(current);

        /* Memory cleanup */
        if (cmd.args_quote_type)
            free(cmd.args_quote_type);
            
        /* ✅ Cleanup heredoc temp files */
        cleanup_heredoc_temp_files(shell);
    }
    else if (current->type == NODE_PIPE)
    {
        /* ✅ CRITICAL: Pre-process ALL heredocs BEFORE forking */
        if (preprocess_all_heredocs(current, env_list, shell) != 0)
        {
            shell->exit_code = 1;
            cleanup_heredoc_temp_files(shell);
            return;
        }

        /* ✅ Now execute pipe with heredocs ready */
        shell->exit_code = execute_pipe(current, *env_list, shell, node);
        
        /* ✅ Cleanup heredoc temp files after pipe execution */
        cleanup_heredoc_temp_files(shell);
    }
    else if (current->type == NODE_REDIR)
    {
        shell->exit_code = execute_redirection(current, env_list, shell);
    }
}