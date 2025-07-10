/* FIXED parse.c - Bash-compliant redirection: correctly handles mixed args and redirections */
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

/* FIXED: free_ast() - Clean up new redirection array */
void free_ast(t_ast *node)
{
    if (!node)
        return;

    // ✅ Clean up NEW redirection array
    if (node->redirections)
    {
        for (int i = 0; i < node->redir_count; i++)
        {
            if (node->redirections[i].filename)
                free(node->redirections[i].filename);
        }
        free(node->redirections);
        node->redirections = NULL;
    }

    // ✅ Clean up existing fields
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

    // Clean up old redirection arrays (for backward compatibility)
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
    
    // Clean up quote arrays
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

    // Clean up child nodes recursively
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

    // Free the node itself
    free(node);
}

/* ADDITIONAL: Güvenli parse_tokens wrapper */
t_ast *parse_tokens(t_token **tokens)
{
	int i = 0;

	if (!tokens || !tokens[0])
		return NULL;

	// Token dizisinde PIPE arıyoruz
	while (tokens[i])
	{
		if (tokens[i]->type == T_PIPE)
			return parse_pipe(tokens, i); // İlk PIPE bulunduğunda parçala
		i++;
	}

	// PIPE yoksa tüm aralık komut olarak gönder
	return parse_command(tokens, 0, i);
}

t_ast *parse_pipe(t_token **tokens, int pipe_index)
{
	t_ast *left = NULL;
	t_ast *right = NULL;
	t_ast *node = NULL;

	// Sol kısmı parse et
	left = parse_command(tokens, 0, pipe_index);
	if (!left)
		return NULL;

	// Sağ kısmı parse et (pipe'dan sonraki tokenlar)
	right = parse_tokens(&tokens[pipe_index + 1]);
	if (!right)
	{
		free_ast(left); // Sol kısmı temizle
		return NULL;
	}

	// Pipe node'unu oluştur
	node = calloc(1, sizeof(t_ast));
	if (!node)
	{
		free_ast(left); // Her iki kısmı da temizle
		free_ast(right);
		return NULL;
	}

	// PIPE AST düğümü oluştur - tüm alanları initialize et
	node->type = NODE_PIPE;
	node->args = NULL;
	node->quote_types = NULL;
	node->redirect_type = 0;
	node->file = NULL;
	node->file_quote = Q_NONE;
	node->left = left;
	node->right = right;
	
	// Redirection arrays'leri initialize et
	node->input_files = NULL;
	node->output_files = NULL;
	node->append_files = NULL;
	node->heredoc_delims = NULL;
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

/* Helper function to add redirection to arrays */
int add_redirection(char ***files, t_quote_type **quotes, int *count, 
                    const char *filename, t_quote_type quote_type)
{
	char **new_files;
	t_quote_type *new_quotes;

	// +2: biri yeni eleman, biri NULL sonlandırma için
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

	(*files)[*count] = NULL;  // ✅ NULL SONLANDIRMA
	return 1;
}

t_ast *parse_command(t_token **tokens, int start, int end)
{
    t_ast *cmd_node = NULL;
    char **args = NULL;
    t_quote_type *quotes = NULL;
    t_redirection *redirections = NULL;
    int i = start;
    int arg_count = 0;
    int redir_count = 0;

    // Create command node first
    cmd_node = malloc(sizeof(t_ast));
    if (!cmd_node)
        return NULL;
        
    // Initialize all fields
    cmd_node->type = NODE_COMMAND;
    cmd_node->args = NULL;
    cmd_node->quote_types = NULL;
    cmd_node->redirect_type = 0;
    cmd_node->file = NULL;
    cmd_node->file_quote = Q_NONE;
    cmd_node->left = NULL;
    cmd_node->right = NULL;
    
    // Initialize NEW redirection tracking
    cmd_node->redirections = NULL;
    cmd_node->redir_count = 0;
    
    // Initialize old redirection arrays for backward compatibility
    cmd_node->input_files = NULL;
    cmd_node->output_files = NULL;
    cmd_node->append_files = NULL;
    cmd_node->heredoc_delims = NULL;
    cmd_node->input_quotes = NULL;
    cmd_node->output_quotes = NULL;
    cmd_node->append_quotes = NULL;
    cmd_node->heredoc_quotes = NULL;
    cmd_node->input_count = 0;
    cmd_node->output_count = 0;
    cmd_node->append_count = 0;
    cmd_node->heredoc_count = 0;

    // ✅ PASS 1: Count arguments and redirections
    i = start;
    while (i < end)
    {
        if (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC)
        {
            redir_count++;
            i += 2; // Skip operator and filename
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

    // ✅ PASS 2: Allocate arrays
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
    }

    // ✅ PASS 3: Fill arrays in ORDER
    int arg_index = 0;
    int redir_index = 0;
    int position = 0;  // Track position in command line
    
    i = start;
    while (i < end)
    {
        if (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC)
        {
            // Handle redirection - PRESERVE ORDER
            if (i + 1 >= end || tokens[i + 1]->type != T_WORD)
            {
                printf("Syntax Error: Missing file after redirection\n");
                // Cleanup
                while (--arg_index >= 0)
                    free(args[arg_index]);
                free(args);
                free(quotes);
                for (int j = 0; j < redir_index; j++)
                    free(redirections[j].filename);
                free(redirections);
                free_ast(cmd_node);
                return NULL;
            }

            // ✅ CRITICAL: Store redirection in ORDER
            redirections[redir_index].type = tokens[i]->type;
            redirections[redir_index].filename = ft_strdup(tokens[i + 1]->value);
            redirections[redir_index].quote_type = tokens[i + 1]->quote_type;
            redirections[redir_index].position = position;
            
            if (!redirections[redir_index].filename)
            {
                // Cleanup on failure
                while (--arg_index >= 0)
                    free(args[arg_index]);
                free(args);
                free(quotes);
                for (int j = 0; j < redir_index; j++)
                    free(redirections[j].filename);
                free(redirections);
                free_ast(cmd_node);
                return NULL;
            }
            
            redir_index++;
            i += 2; // Skip operator and filename
            position += 2;
        }
        else if (tokens[i]->type == T_WORD)
        {
            // Handle command argument
            args[arg_index] = ft_strdup(tokens[i]->value);
            if (!args[arg_index])
            {
                // Cleanup on failure
                while (--arg_index >= 0)
                    free(args[arg_index]);
                free(args);
                free(quotes);
                for (int j = 0; j < redir_index; j++)
                    free(redirections[j].filename);
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
    
    // ✅ Finalize arrays
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
                free(redirections[j].filename);
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

void execute_ast(t_ast *node, t_env **env_list, t_shell *shell)
{
    t_cmd cmd;
    t_ast *current = node;

    while (current)
    {
        if (!current)
            break;

        if (current->type == NODE_COMMAND)
        {
            // t_cmd yapısını hazırla
            ft_bzero(&cmd, sizeof(t_cmd));
            cmd.args = current->args;

            // Quote_types'ları kopyala
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

            /* Redirections setup */
            int redir_result = setup_redirections(current, env_list, shell);
            if (redir_result != 0)
            {
                shell->exit_code = 1;
                if (cmd.args_quote_type)
                    free(cmd.args_quote_type);
                break;
            }

            /* ✅ OPTIMIZED: Ortak fonksiyon kullan */
            shell->exit_code = execute_command_common(*env_list, &cmd, shell, 0);

            /* Redirections restore */
            restore_redirections(shell);

            // Memory cleanup
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);

            break;
        }
        else if (current->type == NODE_PIPE)
        {
            shell->exit_code = execute_pipe(current, *env_list, shell, node);
            break;
        }
        else if (current->type == NODE_REDIR)
        {
            shell->exit_code = execute_redirection(current, env_list, shell);
            break;
        }
        else
        {
            break;
        }
    }
}