#include "minishell.h"
/* Token türünü string olarak döner */
const char	*get_token_type_str(int type)
{
	if (type == T_WORD)
		return ("WORD");
	if (type == T_PIPE)
		return ("PIPE");
	if (type == T_INPUT)
		return ("INPUT");
	if (type == T_OUTPUT)
		return ("OUTPUT");
	if (type == T_APPEND)
		return ("APPEND");
	if (type == T_HEREDOC)
		return ("HEREDOC");
	if (type == T_INVALID)
		return ("INVALID");
	return ("UNKNOWN");
}

/* Token dizisini yazdır */
void	print_tokens(t_token **tokens)
{
	int	i = 0;
	while (tokens[i])
	{
		printf("Token[%d]: '%s' (type: %s, quote: %d)\n",
			i, tokens[i]->value, get_token_type_str(tokens[i]->type), tokens[i]->quote_type);
		i++;
	}
	printf("-----------------\n\n");
}

/* AST ağacını yazdır */

void	print_indent(int level)
{
	while (level-- > 0)
		printf("  ");
}

void	print_ast(t_ast *node, int level)
{
	if (!node)
		return;

	print_indent(level);

	// Komut düğümü
	if (node->type == NODE_COMMAND)
	{
		printf("COMMAND:");
		if (node->args)
		{
			for (int i = 0; node->args[i]; i++)
				printf(" %s", node->args[i]);
		}
		printf("\n");
	}

	// PIPE düğümü
	else if (node->type == NODE_PIPE)
	{
		printf("PIPE\n");

		print_indent(level);
		printf("LEFT:\n");
		print_ast(node->left, level + 1);

		print_indent(level);
		printf("RIGHT:\n");
		print_ast(node->right, level + 1);
	}

	// Redirect düğümü
	else if (node->type == NODE_REDIR)
	{
		printf("REDIR ");
		if (node->redirect_type == REDIR_IN)
			printf("<");
		else if (node->redirect_type == REDIR_OUT)
			printf(">");
		else if (node->redirect_type == REDIR_APPEND)
			printf(">>");
		else if (node->redirect_type == REDIR_HEREDOC)
			printf("<<");

		printf(" file: %s\n", node->file);

		print_indent(level);
		printf("COMMAND/REDIR BELOW:\n");
		print_ast(node->left, level + 1); // Altındaki komutu veya iç içe rediri yazdır
	}
}




/* ... print_tokens, print_ast vs. aynı kalıyor ... */

static int	check_quotes(char *input)
{
	int	in_single = 0;
	int	in_double = 0;
	int	i = 0;

	while (input[i])
	{
		if (input[i] == '"' && !in_single)
			in_double = !in_double;
		else if (input[i] == '\'' && !in_double)
			in_single = !in_single;
		i++;
	}
	if (in_double || in_single)
	{
		printf("Syntax Error: Unmatched quotes\n");
		return (0);
	}
	return (1);
}

/* Komut girişini işleme */
static void	process_input(char *input, t_env *env_list, t_shell *shell)
{
	t_token		**tokens;
	t_ast		*ast;

	if (!check_quotes(input))
		return;

	tokens = tokenize(input);
	if (!tokens)
		return;

	printf("\n--- TOKENS ---\n");
	print_tokens(tokens);

	ast = parse_tokens(tokens);
	expand_ast(ast , env_list, shell);
	execute_ast(ast, &env_list, shell);

	if (ast)
	{
		printf("\n--- AST YAPISI ---\n");
		print_ast(ast, 0);
		free_ast(ast);
	}
	else
		printf("Parser: AST oluşturulamadı!\n");

	freetokens(tokens);
}


/* exit komutunu kontrol etme */
static int	handle_exit(char *input, t_env *env_list)
{
	if (ft_strcmp(input, "exit") == 0)
	{
		write(1, "exit\n", 5);
		free(input);
		free_env_list(env_list);
		return (1);
	}
	return (0);
}

int	main(int argc, char **argv, char **envp)
{
	char		*input;
	t_env		*env_list;
	t_shell		shell;

	(void)argv;
	if (argc != 1)
	{
		write(1, "Usage: ./minishell\n", 19);
		return (1);
	}
	env_list = init_env_list(envp);
	if (!env_list)
	{
		ft_putstr_fd("Error: Failed to initialize environment variables\n", 2);
		return (1);
	}
	shell.exit_code = 0;
	shell.envp = envp;

	while (1)
	{
		input = readline("🐣🌞 minishell ");
		if (!input)
			break ;
		if (*input)
		{
			add_history(input);
			if (handle_exit(input, env_list))
				break ;
			process_input(input, env_list, &shell);
		}
		free(input);
	}
	return (shell.exit_code);
}

