#include "minishell.h"
/* Token türünü string olarak döner */
const char *get_token_type_str(int type)
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
void print_tokens(t_token **tokens)
{
	int i = 0;
	while (tokens[i])
	{
		printf("Token[%d]: '%s' (type: %s, quote: %d)\n",
			   i, tokens[i]->value, get_token_type_str(tokens[i]->type), tokens[i]->quote_type);
		i++;
	}
	printf("-----------------\n\n");
}

/* AST ağacını yazdır */

void print_indent(int level)
{
	while (level-- > 0)
		printf("  ");
}

void print_ast(t_ast *node, int level)
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

static int check_quotes(char *input)
{
	int in_single = 0;
	int in_double = 0;
	int i = 0;

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
static void process_input(char *input, t_env *env_list, t_shell *shell)
{
	t_token **tokens = NULL;
	t_ast *ast = NULL;

	if (!input || !*input)
		return;
	if (!check_quotes(input))
		return;
	tokens = tokenize(input);
	if (!tokens)
		return;
	ast = parse_tokens(tokens);
	freetokens(tokens);
	tokens = NULL;
	if (!ast)
		return;
	expand_ast(ast, env_list, shell);
	//print_ast(ast, 0); // AST'yi yazdırma
	execute_ast(ast, &env_list, shell);
	if (ast)
	{
		free_ast(ast);
		ast = NULL;
	}
}

static int handle_exit(char *input, t_env *env_list)
{
	(void)env_list; // Suppress unused parameter warning
	if (ft_strcmp(input, "exit") == 0)
	{
		write(1, "exit\n", 5);
		free(input);
		// Don't free env_list here - it will be freed in main()
		return (1);
	}
	return (0);
}

int main(int argc, char **argv, char **envp)
{
	char *input;
	t_env *env_list;
	t_shell shell;

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
	shell.saved_stdin = -1;     // Initialize to -1 (not saved)
	shell.saved_stdout = -1;    // Initialize to -1 (not saved)
	shell.heredoc_counter = 0;  // Initialize heredoc counter
	
	while (1)
	{
		set_signal_mode(SIGMODE_PROMPT, &shell); // ← Ctrl+C için PROMPT sinyalleri burada ayarlanır

		input = readline("🐣🌞 minishell ");
		if (!input)
		{
			write(1, "exit\n", 5); // ✅ Ctrl+D için mesaj
			break;
		}
		if (*input)
		{
			add_history(input);
			if (handle_exit(input, env_list))
				break;
			process_input(input, env_list, &shell);
		}
		free(input);
	}
	free_env_list(env_list);
	return (shell.exit_code);
}
