/* FIXED parse.c - Memory leak resolution */
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

	// PIPE AST düğümü oluştur
	node->type = NODE_PIPE;
	node->args = NULL;
	node->quote_types = NULL;
	node->redirect_type = 0;
	node->file = NULL;
	node->file_quote = Q_NONE;
	node->left = left;
	node->right = right;

	return node;
}

/* FIXED: Komut + redirect'leri ayrıştıran fonksiyon - Memory leak düzeltildi */
t_ast *parse_command(t_token **tokens, int start, int end)
{
	t_ast *cmd_node = NULL;
	t_ast *redir_node = NULL;
	char **args = NULL;
	t_quote_type *quotes = NULL;
	int i = start;
	int count = 0;

	// 1. Komut argümanlarının sayısını belirle (başlangıç WORD'ler)
	while (i < end && tokens[i]->type == T_WORD)
	{
		count++;
		i++;
	}

	// 2. Argümanlar ve quote tipleri için yer ayır
	if (count > 0)
	{
		args = malloc(sizeof(char *) * (count + 1));
		quotes = malloc(sizeof(t_quote_type) * count);
		if (!args || !quotes)
		{
			// ✅ FIX: Proper cleanup on allocation failure
			free(args);
			free(quotes);
			return NULL;
		}

		i = start;
		int j = 0;
		while (i < end && tokens[i]->type == T_WORD)
		{
			args[j] = ft_strdup(tokens[i]->value);
			if (!args[j]) // ✅ FIX: Check strdup failure
			{
				// Free previously allocated strings
				while (--j >= 0)
					free(args[j]);
				free(args);
				free(quotes);
				return NULL;
			}
			quotes[j] = tokens[i]->quote_type;
			i++;
			j++;
		}
		args[j] = NULL;

		// 3. Komut node'u oluştur
		cmd_node = malloc(sizeof(t_ast));
		if (!cmd_node)
		{
			// ✅ FIX: Cleanup on node allocation failure
			free_args_array(args);
			free(quotes);
			return NULL;
		}

		// ✅ INITIALIZE all fields properly
		cmd_node->type = NODE_COMMAND;
		cmd_node->args = args;
		cmd_node->quote_types = quotes;
		cmd_node->redirect_type = 0;
		cmd_node->file = NULL;
		cmd_node->file_quote = Q_NONE;
		cmd_node->left = NULL;
		cmd_node->right = NULL;
	}

	// 4. Redirect'leri sırayla işle
	while (i < end && (tokens[i]->type >= T_INPUT && tokens[i]->type <= T_HEREDOC))
	{
		t_redirect_type redir_type;

		// Redirection türünü belirle
		if (tokens[i]->type == T_INPUT)
			redir_type = REDIR_IN;
		else if (tokens[i]->type == T_OUTPUT)
			redir_type = REDIR_OUT;
		else if (tokens[i]->type == T_APPEND)
			redir_type = REDIR_APPEND;
		else
			redir_type = REDIR_HEREDOC;

		// Redir sonrası dosya adı gelmeli
		if (i + 1 >= end || tokens[i + 1]->type != T_WORD)
		{
			printf("Syntax Error: Missing file after redirection\n");
			free_ast(cmd_node);
			return NULL;
		}

		// 5. Redir node oluştur
		redir_node = malloc(sizeof(t_ast));
		if (!redir_node)
		{
			free_ast(cmd_node);
			return NULL;
		}

		// ✅ FIX: Check strdup failure
		redir_node->file = ft_strdup(tokens[i + 1]->value);
		if (!redir_node->file)
		{
			free(redir_node);
			free_ast(cmd_node);
			return NULL;
		}

		// ✅ INITIALIZE all fields properly
		redir_node->type = NODE_REDIR;
		redir_node->redirect_type = redir_type;
		redir_node->file_quote = tokens[i + 1]->quote_type;
		redir_node->args = NULL;
		redir_node->quote_types = NULL;
		redir_node->left = cmd_node;
		redir_node->right = NULL;

		cmd_node = redir_node;
		i += 2;
	}

	// 6. Eğer redirect başta geldi ve komut henüz yoksa, şimdi al
	if (cmd_node && cmd_node->type == NODE_REDIR && cmd_node->left == NULL)
	{
		int k = i;
		int cmd_start = -1;

		while (k < end)
		{
			if (tokens[k]->type == T_WORD)
			{
				cmd_start = k;
				break;
			}
			k++;
		}

		if (cmd_start != -1)
		{
			t_ast *recovered_cmd = parse_command(tokens, cmd_start, end);
			if (!recovered_cmd)
			{
				free_ast(cmd_node);
				return NULL;
			}
			cmd_node->left = recovered_cmd;
		}
	}

	return cmd_node;
}

/* Parse.c'de execute_ast fonksiyonunda küçük düzeltme */

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

            if (is_builtin(&cmd))
                shell->exit_code = run_builtin(*env_list, &cmd, shell);
            else
                shell->exit_code = execute_command(*env_list, &cmd, shell);

            // Bellek temizliği
            if (cmd.args_quote_type)
                free(cmd.args_quote_type);

            break;
        }
        else if (current->type == NODE_PIPE)
        {
            // ✅ FIX: Root AST'yi korumak için node'u geçir
            shell->exit_code = execute_pipe(current, *env_list, shell, node);
            break;
        }
        else if (current->type == NODE_REDIR)
        {
            // Redirection'ı execute et
            shell->exit_code = execute_redirection(current, env_list, shell);
            break;
        }
        else
        {
            break;
        }
    }
}
