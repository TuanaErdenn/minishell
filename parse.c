#include "minishell.h"

/* PIPE varsa ayrıştır, yoksa komut olarak işle */
t_ast *parse_tokens(t_token **tokens)
{
	int i = 0;

	// Token dizisinde PIPE arıyoruz
	while (tokens[i])
	{
		if (tokens[i]->type == T_PIPE)
			return parse_pipe(tokens, i); // İlk PIPE bulunduğunda parçala
		i++;
	}
	return parse_command(tokens, 0, i); // PIPE yoksa tüm aralık komut olarak gönder
}

/* PIPE bulunduğunda sol ve sağ parçaları ayırarak AST'yi rekürsif kur */
t_ast *parse_pipe(t_token **tokens, int pipe_index)
{
	t_ast *node;
	t_ast *left = parse_command(tokens, 0, pipe_index); // PIPE'dan önceki kısım
	t_ast *right = parse_tokens(&tokens[pipe_index + 1]); // Sonraki kısım rekürsif çağrılır

	node = malloc(sizeof(t_ast));
	if (!node)
		return NULL;

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

/* Komut + redirect’leri ayrıştıran fonksiyon */
t_ast *parse_command(t_token **tokens, int start, int end)
{
	t_ast *cmd_node = NULL;
	t_ast *redir_node = NULL;
	char **args = NULL;
	t_quote_type *quotes = NULL;
	int i = start;
	int count = 0;

	// 1. Komut argümanlarının sayısını belirle (başlangıç WORD’ler)
	while (i < end && tokens[i]->type == T_WORD)
	{
		count++;
		i++;
	}

	// 2. Argümanlar ve quote tipleri için yer ayır
	if (count > 0)
	{
		args = malloc(sizeof(char *) * (count + 1));
		quotes = malloc(sizeof(int) * count);
		if (!args || !quotes)
			return (free(args), free(quotes), NULL);

		i = start;
		int j = 0;
		while (i < end && tokens[i]->type == T_WORD)
		{
			args[j] = ft_strdup(tokens[i]->value);
			quotes[j] = tokens[i]->quote_type;
			i++;
			j++;
		}
		args[j] = NULL;

		// 3. Komut node'u oluştur
		cmd_node = malloc(sizeof(t_ast));
		if (!cmd_node)
			return (free(args), free(quotes), NULL);
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
			return (free_ast(cmd_node), NULL);
		redir_node->type = NODE_REDIR;
		redir_node->redirect_type = redir_type;
		redir_node->file = ft_strdup(tokens[i + 1]->value);
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


/* AST'nin belleğini temizler (rekürsif olarak) */
void	free_ast(t_ast *node)
{
	int	i;

	if (!node)
		return;
	if (node->args)
	{
		i = 0;
		while (node->args[i])
			free(node->args[i++]);
		free(node->args);
	}
	free(node->quote_types);
	free(node->file);
	free_ast(node->left);
	free_ast(node->right);
	free(node);
}
