#include "minishell.h"

void	free_str_array(char **arr)
{
	int	i = 0;
	if (!arr)
		return;
	while (arr[i])
		free(arr[i++]);
	free(arr);
}

/* '$' ile başlayan kelimelerdeki değişkenin karşılığını bulur */
char *expand_variable(const char *str, t_env *env_list, t_shell *shell)
{
	char	*key;
	char	*value;
	int		i;

	if (!str || str[0] != '$')
		return (ft_strdup(str));

	if (str[1] == '?')
		return (ft_itoa(shell->exit_code));

	i = 1;
	while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
		i++;
	key = ft_substr(str, 1, i - 1);
    if (!key)
       return (NULL);
    value = get_env_value(env_list, key);
    free(key);

	if (!value)
		return (ft_strdup(""));
	return (ft_strdup(value));
}

char *expand_string_with_vars(const char *str, t_env *env_list, t_shell *shell)
{
	char	*result = ft_strdup("");
	int		i = 0;

	while (str[i])
	{
		if (str[i] == '$')
		{
			if (str[i + 1] == '?')
			{
				char *exit_str = ft_itoa(shell->exit_code);
				char *tmp = ft_strjoin(result, exit_str);
				free(result);
				result = tmp;
				free(exit_str);
				i += 2;
			}
			else
			{
				int	start = i + 1;
				while (str[start] && (ft_isalnum(str[start]) || str[start] == '_'))
					start++;
                char *key = ft_substr(str, i + 1, start - (i + 1));
                if (!key)
                {
                        free(result);
                        return (NULL);
                }
                char *val = get_env_value(env_list, key);
                char *tmp = ft_strjoin(result, val ? val : "");
                free(result);
                result = tmp;
                free(key);
                i = start;				
			}
		}
		else
		{
			char temp[2] = {str[i], 0};
			char *tmp = ft_strjoin(result, temp);
			free(result);
			result = tmp;
			i++;
		}
	}
	return (result);
}

char **expand_args(char **args, t_quote_type *quote_types, t_env *env_list, t_shell *shell)
{
	int		i;
	char	**new_args;

	if (!args)
		return (NULL);
	i = 0;
	while (args[i])
		i++;
	new_args = malloc(sizeof(char *) * (i + 1));
	if (!new_args)
		return (NULL);
	i = 0;
	while (args[i])
	{
		if (quote_types && quote_types[i] == Q_SINGLE)
			new_args[i] = ft_strdup(args[i]); // tek tırnakta expand yok
		else
			new_args[i] = expand_string_with_vars(args[i], env_list, shell);
		i++;
	}
	new_args[i] = NULL;
	return (new_args);
}

void expand_ast(t_ast *node, t_env *env_list, t_shell *shell)
{
	if (!node)
		return ;
	if (node->type == NODE_COMMAND && node->args)
	{
		char **expanded = expand_args(node->args, node->quote_types, env_list, shell);
		free_str_array(node->args);
		node->args = expanded;
	}
	expand_ast(node->left, env_list, shell);
	expand_ast(node->right, env_list, shell);
}
