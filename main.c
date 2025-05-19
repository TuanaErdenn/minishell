#include "minishell.h"

void	print_ast(t_astnode *node, int level)
{
	if (!node)
		return ;
	for (int i = 0; i < level; i++)
		printf("  ");
	printf("%s\n", node->content);
	print_ast(node->left, level + 1);
	print_ast(node->right, level + 1);
}

/* Token türünü string olarak döner */
const char	*get_token_type_str(int type)
{
	if (type == 0)
		return ("WORD");
	if (type == 1)
		return ("PIPE");
	if (type == 2)
		return ("REDIR_IN");
	if (type == 3)
		return ("REDIR_OUT");
	if (type == 4)
		return ("APPEND");
	if (type == 5)
		return ("HEREDOC");
	return ("UNKNOWN");
}

/* Token dizisini yazdır */
void	print_tokens(t_token **tokens)
{
	int	i;

	i = 0;
	while (tokens[i])
	{
		printf("Token[%d]: '%s' (type: %s)\n", i, tokens[i]->value,
			get_token_type_str(tokens[i]->type));
		i++;
	}
	printf("-----------------\n\n");
}

void	print_env(char **envp)
{
	int	i;

	i = 0;
	while (envp[i])
	{
		printf("%s\n", envp[i]);
		i++;
	}
}

/* Çevre değişkenlerini yazdır (debug için) */
void	print_env_list(t_env *env_list)
{
	t_env	*temp;

	temp = env_list;
	while (temp)
	{
		printf("%s=%s\n", temp->key, temp->value);
		temp = temp->next;
	}
}

static int	check_quotes(char *input)
{
	int	in_single;
	int	in_double;
	int	i;

	in_single = 0;
	in_double = 0;
	i = 0;
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

/* AST'yi yürütme (şu an sadece debug için yazdırma işlemi yapacak) */
static int	execute_ast(t_astnode *ast, t_env *env_list, int exit_code)
{
	if (!ast)
		return (exit_code);
	printf("Yürütülüyor: %s\n", ast->content);
	// Burada ileride komut yürütme işlemi yapılacak
	if (ast->left)
		execute_ast(ast->left, env_list, exit_code);
	if (ast->right)
		execute_ast(ast->right, env_list, exit_code);
	return (exit_code);
}

/* Komut girişini işleme */
static int	process_input(char *input, t_env *env_list, int exit_code)
{
	t_token		**tokens;
	t_astnode	*ast;

	if (!check_quotes(input))
		return (exit_code);
	tokens = tokenize(input);
	if (!tokens)
		return (exit_code);
	printf("\n--- TOKENS ---\n");
	print_tokens(tokens);
	ast = parse(tokens);
	if (ast)
	{
		printf("\n--- AST YAPISI ---\n");
		print_ast(ast, 0);
		// AST'yi Yürütme
		exit_code = execute_ast(ast, env_list, exit_code);
		free_ast(ast);
	}
	// Bellek Temizliği
	freetokens(tokens);
	return (exit_code);
}

/* exit komutunu kontrol etme */
static int	handle_exit(char *input, t_env *env_list)
{
	if (strcmp(input, "exit") == 0)
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
	char	*input;
	t_env	*env_list;
	int		exit_code;

	exit_code = 0;
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
			exit_code = process_input(input, env_list, exit_code);
		}
		free(input);
	}
	return (exit_code);
}
