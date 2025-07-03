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

/* ✅ ENHANCED: AST'nin belleğini temizler (rekürsif olarak) */
void free_ast(t_ast *node)
{
	if (!node)
		return;

	// ✅ ENHANCED: Args dizisini güvenli temizle
	if (node->args)
	{
		free_args_array(node->args);
		node->args = NULL;
	}

	// ✅ ENHANCED: Quote types dizisini temizle
	if (node->quote_types)
	{
		free(node->quote_types);
		node->quote_types = NULL;
	}

	// ✅ NEW: Redirection arrays'leri temizle
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
	
	// Quote arrays'leri temizle
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

	// ✅ ENHANCED: File string'ini temizle
	if (node->file)
	{
		free(node->file);
		node->file = NULL;
	}

	// ✅ CRITICAL: Sol ve sağ child'ları özyinelemeli temizle
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

	// ✅ FINAL: Node'un kendisini temizle
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
	node = malloc(sizeof(t_ast));
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
static int add_redirection(char ***files, t_quote_type **quotes, int *count, 
                          const char *filename, t_quote_type quote_type)
{
	char **new_files;
	t_quote_type *new_quotes;

	// Realloc arrays
	new_files = realloc(*files, sizeof(char*) * (*count + 1));
	new_quotes = realloc(*quotes, sizeof(t_quote_type) * (*count + 1));
	
	if (!new_files || !new_quotes)
	{
		free(new_files);
		free(new_quotes);
		return 0;
	}
	
	*files = new_files;
	*quotes = new_quotes;
	
	// Add new entry
	(*files)[*count] = ft_strdup(filename);
	if (!(*files)[*count])
		return 0;
		
	(*quotes)[*count] = quote_type;
	(*count)++;
	return 1;
}

/* ✅ FIXED: Bash-compliant command parsing - arguments can appear anywhere */
t_ast *parse_command(t_token **tokens, int start, int end)
{
	t_ast *cmd_node = NULL;
	char **args = NULL;
	t_quote_type *quotes = NULL;
	int i = start;
	int arg_count = 0;

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
	
	// Initialize redirection arrays
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

	// ✅ PASS 1: Count command arguments (skip redirection operators AND their files)
	i = start;
	while (i < end)
	{
		if (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC)
		{
			// Skip redirection operator AND its file
			i += 2; // Skip both operator and filename
			if (i > end)
			{
				printf("Syntax Error: Missing file after redirection\n");
				free_ast(cmd_node);
				return NULL;
			}
		}
		else if (tokens[i]->type == T_WORD)
		{
			// This is a command argument
			arg_count++;
			i++;
		}
		else
		{
			i++;
		}
	}

	// ✅ PASS 2: Collect arguments and redirections
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

		int arg_index = 0;
		i = start;
		while (i < end)
		{
			if (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC)
			{
				// Handle redirection operator + filename pair
				if (i + 1 >= end || tokens[i + 1]->type != T_WORD)
				{
					printf("Syntax Error: Missing file after redirection\n");
					// Cleanup
					while (--arg_index >= 0)
						free(args[arg_index]);
					free(args);
					free(quotes);
					free_ast(cmd_node);
					return NULL;
				}

				// Add redirection based on operator type
				t_token_type redir_type = tokens[i]->type;
				char *filename = tokens[i + 1]->value;
				t_quote_type file_quote = tokens[i + 1]->quote_type;

				if (redir_type == T_INPUT)
				{
					if (!add_redirection(&cmd_node->input_files, &cmd_node->input_quotes,
									   &cmd_node->input_count, filename, file_quote))
					{
						// Cleanup on failure
						while (--arg_index >= 0)
							free(args[arg_index]);
						free(args);
						free(quotes);
						free_ast(cmd_node);
						return NULL;
					}
				}
				else if (redir_type == T_OUTPUT)
				{
					if (!add_redirection(&cmd_node->output_files, &cmd_node->output_quotes,
									   &cmd_node->output_count, filename, file_quote))
					{
						// Cleanup on failure
						while (--arg_index >= 0)
							free(args[arg_index]);
						free(args);
						free(quotes);
						free_ast(cmd_node);
						return NULL;
					}
				}
				else if (redir_type == T_APPEND)
				{
					if (!add_redirection(&cmd_node->append_files, &cmd_node->append_quotes,
									   &cmd_node->append_count, filename, file_quote))
					{
						// Cleanup on failure
						while (--arg_index >= 0)
							free(args[arg_index]);
						free(args);
						free(quotes);
						free_ast(cmd_node);
						return NULL;
					}
				}
				else // HEREDOC
				{
					if (!add_redirection(&cmd_node->heredoc_delims, &cmd_node->heredoc_quotes,
									   &cmd_node->heredoc_count, filename, file_quote))
					{
						// Cleanup on failure
						while (--arg_index >= 0)
							free(args[arg_index]);
						free(args);
						free(quotes);
						free_ast(cmd_node);
						return NULL;
					}
				}
				
				// Skip both operator and filename
				i += 2;
			}
			else if (tokens[i]->type == T_WORD)
			{
				// This is a command argument
				args[arg_index] = ft_strdup(tokens[i]->value);
				if (!args[arg_index])
				{
					// Cleanup on failure
					while (--arg_index >= 0)
						free(args[arg_index]);
					free(args);
					free(quotes);
					free_ast(cmd_node);
					return NULL;
				}
				quotes[arg_index] = tokens[i]->quote_type;
				arg_index++;
				i++;
			}
			else
			{
				i++;
			}
		}
		
		args[arg_count] = NULL;
		cmd_node->args = args;
		cmd_node->quote_types = quotes;
	}

	return cmd_node;
}

/* parse.c - execute_ast() function with updated restore_redirections */

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
			// t_cmd yapısını tamamen doldur
			ft_bzero(&cmd, sizeof(t_cmd));
			cmd.args = current->args;

			// Quote_types'ları t_cmd'e kopyala
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

			// ✅ Setup redirections
			int redir_result = setup_redirections(current, env_list, shell);

			// ✅ BASH BEHAVIOR: Redirection başarısızsa komut çalıştırma!
			if (redir_result != 0)
			{
				shell->exit_code = 1;
				// Bellek temizliği
				if (cmd.args_quote_type)
					free(cmd.args_quote_type);
				// ✅ UPDATED: restore_redirections now takes shell parameter
				restore_redirections(shell);
				break;
			}

			// ✅ Redirection başarılıysa komut çalıştır
			if (is_builtin(&cmd))
				shell->exit_code = run_builtin(*env_list, &cmd, shell);
			else
				shell->exit_code = execute_command(*env_list, &cmd, shell);

			// ✅ UPDATED: restore_redirections now takes shell parameter
			restore_redirections(shell);

			// Bellek temizliği
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
			// Eski redirection sistemi - artık kullanılmayacak
			shell->exit_code = execute_redirection(current, env_list, shell);
			break;
		}
		else
		{
			break;
		}
	}
}