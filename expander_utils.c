/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 18:41:13 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/13 20:19:05 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


// Helper function: Handle $? expansion
char *handle_exit_code_expansion(char **result, t_shell *shell)
{
	char *exit_str;
	char *tmp;

	exit_str = ft_itoa(shell->exit_code);
	if (!exit_str)
		return (NULL);
	tmp = ft_strjoin(*result, exit_str);
	free(*result);
	free(exit_str);
	if (!tmp)
		return (NULL);
	*result = tmp;
	return (*result);
}

// Helper function: Handle variable expansion
char *handle_var_expansion(char **result, const char *str, int *i, t_env *env_list)
{
	int start;
	char *key;
	char *val;
	char *tmp;

	start = *i + 1;
	while (str[start] && (ft_isalnum(str[start]) || str[start] == '_'))
		start++;
	key = ft_substr(str, *i + 1, start - (*i + 1));
	if (!key)
		return (NULL);
	val = get_env_value(env_list, key);
	tmp = ft_strjoin(*result, val ? val : "");
	free(*result);
	free(key);
	if (!tmp)
		return (NULL);
	*result = tmp;
	*i = start;
	return (*result);
}

// Helper function: Add single character to result
char *add_char_to_result(char **result, char c)
{
	char temp[2];
	char *tmp;

	temp[0] = c;
	temp[1] = '\0';
	tmp = ft_strjoin(*result, temp);
	free(*result);
	if (!tmp)
		return (NULL);
	*result = tmp;
	return (*result);
}

// Helper function: Handle dollar sign expansion
char *handle_dollar_expansion(char **result, const char *str, int *i, 
								t_env *env_list, t_shell *shell)
{
	if (str[*i + 1] == '?')
	{
		if (!handle_exit_code_expansion(result, shell))
			return (NULL);
		*i += 2;
	}
	else if (str[*i + 1] >= '0' && str[*i + 1] <= '9')
	{
		*i += 2;
	}
	else if (str[*i + 1] && (ft_isalpha(str[*i + 1]) || str[*i + 1] == '_'))
	{
		if (!handle_var_expansion(result, str, i, env_list))
			return (NULL);
	}
	else
	{
		if (!add_char_to_result(result, '$'))
			return (NULL);
		(*i)++;
	}
	return (*result);
}

// Main expansion function
char *expand_string_with_vars(const char *str, t_env *env_list, t_shell *shell)
{
	char *result;
	int i;

	if (!str)
		return (NULL);
	if (!ft_strchr(str, '$'))
		return (ft_strdup(str));
	result = ft_strdup("");
	if (!result)
		return (NULL);
	i = 0;
	while (str[i])
	{
		if (str[i] == '$')
		{
			if (!handle_dollar_expansion(&result, str, &i, env_list, shell))
				return (NULL);
		}
		else
		{
			if (!add_char_to_result(&result, str[i]))
				return (NULL);
			i++;
		}
	}
	return (result);
}

/* Token'ın heredoc delimiter olup olmadığını kontrol eder */
int is_heredoc_delimiter(t_token *token)
{
    if (!token || !token->prev)
        return 0;
    
    // Önceki token << (T_HEREDOC) ise bu token heredoc delimiter'dır
    if (token->prev->type == T_HEREDOC)
        return 1;
    
    return 0;
}

/* Token-aware expansion function - heredoc delimiter'larda expansion yapmaz */
char *expand_token_with_vars(t_token *token, t_env *env_list, t_shell *shell)
{
    // Heredoc delimiter ise expansion yapma
    if (is_heredoc_delimiter(token))
        return ft_strdup(token->value);
    
    // Normal expansion yap
    return expand_string_with_vars(token->value, env_list, shell);
}