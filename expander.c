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
	value = get_env_value(env_list, key);
	free(key);

	if (!value)
		return (ft_strdup(""));
	return (ft_strdup(value));
}

/* String içindeki tüm değişkenleri expand eder */
char *expand_string_with_vars(const char *str, t_env *env_list, t_shell *shell)
{
    char *result;
    char *tmp;
    int i = 0;

    if (!str)
        return (NULL);
    
    // $ yoksa direkt kopyala
    if (!ft_strchr(str, '$'))
        return (ft_strdup(str));
    
    result = ft_strdup("");
    if (!result)
        return (NULL);

    while (str[i])
    {
        if (str[i] == '$')
        {
            if (str[i + 1] == '?')
            {
                // $? - exit code
                char *exit_str = ft_itoa(shell->exit_code);
                if (exit_str)
                {
                    tmp = ft_strjoin(result, exit_str);
                    free(result);
                    free(exit_str);
                    if (!tmp)
                        return (NULL);
                    result = tmp;
                }
                i += 2;
            }
            else if (str[i + 1] && (ft_isalpha(str[i + 1]) || str[i + 1] == '_'))
            {
                // $VAR - değişken adı
                int start = i + 1;
                while (str[start] && (ft_isalnum(str[start]) || str[start] == '_'))
                    start++;
                
                char *key = ft_substr(str, i + 1, start - (i + 1));
                if (key)
                {
                    char *val = get_env_value(env_list, key);
                    tmp = ft_strjoin(result, val ? val : "");
                    free(result);
                    free(key);
                    if (!tmp)
                        return (NULL);
                    result = tmp;
                }
                i = start;
            }
            else
            {
                // Tek başına $ - literal olarak ekle
                char temp[2] = {'$', 0};
                tmp = ft_strjoin(result, temp);
                free(result);
                if (!tmp)
                    return (NULL);
                result = tmp;
                i++;
            }
        }
        else
        {
            // Normal karakter
            char temp[2] = {str[i], 0};
            tmp = ft_strjoin(result, temp);
            free(result);
            if (!tmp)
                return (NULL);
            result = tmp;
            i++;
        }
    }
    return (result);
}

/* Args array'ini expand eder - sadece gerektiğinde */
char **expand_args(char **args, t_quote_type *quote_types, t_env *env_list, t_shell *shell)
{
    int i, count;
    int needs_expansion = 0;
    char **new_args;

    if (!args)
        return (NULL);
    
    // Argüman sayısını hesapla
    count = 0;
    while (args[count])
        count++;
    
    // Gerçekten expansion gerekip gerekmediğini kontrol et
    i = 0;
    while (i < count && args[i])
    {
        // Tek tırnak DEĞİLSE ve içinde $ varsa expansion gerekir
        if ((!quote_types || quote_types[i] != Q_SINGLE) && 
            ft_strchr(args[i], '$'))
        {
            needs_expansion = 1;
            break;
        }
        i++;
    }
    
    // Expansion gerekmiyorsa NULL döndür (orijinal args'lar korunur)
    if (!needs_expansion)
        return (NULL);
    
    // Yeni array oluştur
    new_args = malloc(sizeof(char *) * (count + 1));
    if (!new_args)
        return (NULL);
    
    // Her argümanı işle
    i = 0;
    while (i < count && args[i])
    {
        if (quote_types && quote_types[i] == Q_SINGLE)
        {
            // Tek tırnak: literal kopyala, expansion yapma
            new_args[i] = ft_strdup(args[i]);
        }
        else if (ft_strchr(args[i], '$'))
        {
            // $ var ve tek tırnak değil: expansion yap
            new_args[i] = expand_string_with_vars(args[i], env_list, shell);
        }
        else
        {
            // $ yok: direkt kopyala
            new_args[i] = ft_strdup(args[i]);
        }
        
        if (!new_args[i])
        {
            // Hata durumunda temizlik
            while (--i >= 0)
                free(new_args[i]);
            free(new_args);
            return (NULL);
        }
        i++;
    }
    new_args[i] = NULL;
    return (new_args);
}

/* AST'yi özyinelemeli olarak expand eder */
void expand_ast(t_ast *node, t_env *env_list, t_shell *shell)
{
    if (!node)
        return;

    // Command node'ları için args expansion
    if (node->type == NODE_COMMAND && node->args)
    {
        char **old_args = node->args;
        char **expanded = expand_args(node->args, node->quote_types, env_list, shell);
        
        if (expanded != NULL)
        {
            // Expansion yapıldı, eski args'ları değiştir
            node->args = expanded;
            free_str_array(old_args);
        }
        // expanded == NULL: expansion gerekmedi, orijinal args'lar korunur
    }
    
    // Redirection node'ları için file expansion
    if (node->type == NODE_REDIR && node->file && node->file_quote != Q_SINGLE)
    {
        // Sadece $ içeriyorsa expand et
        if (ft_strchr(node->file, '$'))
        {
            char *old_file = node->file;
            char *expanded_file = expand_string_with_vars(node->file, env_list, shell);
            if (expanded_file)
            {
                node->file = expanded_file;
                free(old_file);
            }
        }
    }

    // Alt düğümleri özyinelemeli olarak expand et
    expand_ast(node->left, env_list, shell);
    expand_ast(node->right, env_list, shell);
}