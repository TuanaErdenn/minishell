#include "minishell.h"

void	free_key_value(char *key, char *value)
{
	if (key)
		free(key);
	if (value)
		free(value);
}

// Quote'ları temizleyen fonksiyon - Enhanced with debug
char *remove_quotes(char *str)
{
	int len;
	char *result;
	
	if (!str)
		return (NULL);
	
	len = ft_strlen(str);
	
	// Debug: Print what we're processing
	// printf("DEBUG remove_quotes: input='%s', len=%d\n", str, len);
	
	// Çift tırnak veya tek tırnak kontrolü
	if (len >= 2 && 
		((str[0] == '"' && str[len - 1] == '"') || 
		 (str[0] == '\'' && str[len - 1] == '\'')))
	{
		// Tırnakları çıkararak substring oluştur
		result = ft_substr(str, 1, len - 2);
		// printf("DEBUG remove_quotes: removed quotes, result='%s'\n", result ? result : "NULL");
		return (result);
	}
	
	// Tırnak yoksa orijinal string'i kopyala
	result = ft_strdup(str);
	// printf("DEBUG remove_quotes: no quotes, result='%s'\n", result ? result : "NULL");
	return (result);
}

// Token'ları birleştiren fonksiyon - export için özel
char *merge_export_tokens(char **args, int start_index, int *merged_count)
{
	char *result = NULL;
	char *temp;
	int i = start_index;
	
	if (!args || !args[i])
		return (NULL);
	
	// İlk token'ı kopyala
	result = ft_strdup(args[i]);
	if (!result)
		return (NULL);
	i++;
	*merged_count = 1;
	
	// Eğer ilk token'da = var ve tam olarak = ile bitiyorsa
	// ve sonraki token varsa, birleştir
	int len = ft_strlen(result);
	if (len > 0 && result[len - 1] == '=' && args[i])
	{
		temp = ft_strjoin(result, args[i]);
		free(result);
		if (!temp)
			return (NULL);
		result = temp;
		(*merged_count)++;
	}
	
	return (result);
}

// key value formatını parse et - Quote handling ile
int	parse_export_arg(char *arg, char **key, char **value)
{
	char	*equal;
	char	*temp_key;
	char	*temp_value;

	if (!arg || !key || !value)
		return (1);

	equal = ft_strchr(arg, '=');
	if (!equal)
	{
		// Eşittir işareti yoksa, tüm argüman key olur
		temp_key = remove_quotes(arg);
		if (!temp_key)
			return (1);
		*key = temp_key;
		*value = NULL;
		return (0);
	}
	
	// Key kısmını ayır (eşittir işaretine kadar olan kısım)
	*key = ft_substr(arg, 0, equal - arg);
	if (!*key)
		return (1);
	
	// Key'den quote'ları temizle
	temp_key = remove_quotes(*key);
	free(*key);
	*key = temp_key;
	if (!*key)
		return (1);

	// Value kısmını ayır (eşittir işaretinden sonraki kısım)
	temp_value = ft_strdup(equal + 1);
	if (!temp_value)
	{
		free(*key);
		*key = NULL;
		return (1);
	}
	
	// Value'dan quote'ları temizle
	*value = remove_quotes(temp_value);
	free(temp_value);
	if (!*value)
	{
		free(*key);
		*key = NULL;
		return (1);
	}
	
	return (0);
}

t_env	*create_env_node(char *key, char *value)
{
	t_env	*new;

	if (!key)
		return (NULL);

	new = (t_env *)malloc(sizeof(t_env));
	if (!new)
		return (NULL);

	new->key = ft_strdup(key);
	if (!new->key)
	{
		free(new);
		return (NULL);
	}
	if (value)
	{
		new->value = ft_strdup(value);
		if (!new->value)
		{
			free(new->key);
			free(new);
			return (NULL);
		}
	}
	else
		new->value = NULL;

	new->next = NULL;
	return (new);
}

int	add_update_env(t_env **env_list, char *key, char *value)
{
	t_env	*cur;
	t_env	*new_node;

	if (!env_list || !key)
		return (1);

	cur = *env_list;
	while (cur)
	{
		// update kısmı
		if (ft_strcmp(cur->key, key) == 0)
		{
			if (cur->value)
				free(cur->value);
			if (value)
				cur->value = ft_strdup(value);
			else
				cur->value = NULL;
			return (0);
		}
		cur = cur->next;
	}
	// add kısmı - yeni node lazım
	new_node = create_env_node(key, value);
	if (!new_node)
		return (1);
	add_env_node(env_list, new_node);
	return (0);
}

int	is_valid_identifier(char *key)
{
	int	i;

	if (!key || !*key)
		return (0);

	// İlk karakter alfabetik veya '_' olmalı
	if (!ft_isalpha(key[0]) && key[0] != '_')
		return (0);

	// Diğer karakterler alfanümerik veya '_' olabilir
	i = 1;
	while (key[i])
	{
		if (!ft_isalnum(key[i]) && key[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

void	print_sorted_env(t_env **env_list)
{
	t_env	**sorted_array;
	t_env	*current;
	t_env	*temp;
	int		count;
	int		i;
	int		j;

	if (!env_list || !*env_list)
		return ;
	count = 0;
	current = *env_list;
	while (current && ++count)
		current = current->next;
	sorted_array = (t_env **)malloc(sizeof(t_env *) * count);
	if (!sorted_array)
		return ;
	current = *env_list;
	i = -1;
	while (current)
	{
		sorted_array[++i] = current;
		current = current->next;
	}
	i = -1;
	while (++i < count - 1)
	{
		j = -1;
		while (++j < count - i - 1)
		{
			if (ft_strcmp(sorted_array[j]->key, sorted_array[j + 1]->key) > 0)
			{
				temp = sorted_array[j];
				sorted_array[j] = sorted_array[j + 1];
				sorted_array[j + 1] = temp;
			}
		}
	}
	i = -1;
	while (++i < count)
	{
		if (!ft_strncmp(sorted_array[i]->key, "_", 2))
			continue;
		ft_putstr_fd("declare -x ", 1);
		ft_putstr_fd(sorted_array[i]->key, 1);
		if (sorted_array[i]->value )
		{
			ft_putstr_fd("=\"", 1);
			ft_putstr_fd(sorted_array[i]->value, 1);
			ft_putstr_fd("\"", 1);
		}
		ft_putstr_fd("\n", 1);
	}
	free(sorted_array);
}

// Enhanced process_export_arg with token merging
int	process_export_arg(t_env **env_list, char **args, int i, int *status)
{
	char	*key;
	char	*value;
	char	*merged_arg;
	int		merged_count;

	key = NULL;
	value = NULL;
	merged_count = 0;
	
	// Token'ları birleştir (gerekirse)
	merged_arg = merge_export_tokens(args, i, &merged_count);
	if (!merged_arg)
	{
		ft_putstr_fd("minishell: export: error merging tokens\n", 2);
		*status = 1;
		return (1);
	}
	
	if (parse_export_arg(merged_arg, &key, &value) != 0)
	{
		ft_putstr_fd("minishell: export: error parsing argument\n", 2);
		*status = 1;
		free(merged_arg);
		return (merged_count);
	}
	
	if (!is_valid_identifier(key))
	{
		ft_putstr_fd("minishell: export: `", 2);
		ft_putstr_fd(merged_arg, 2);
		ft_putstr_fd("': not a valid identifier\n", 2);
		*status = 1;
	}
	else if (add_update_env(env_list, key, value) != 0)
		*status = 1;
	
	free_key_value(key, value);
	free(merged_arg);
	return (merged_count);
}

int	execute_export(t_env **env_list, char **args)
{
	int	i;
	int	status;
	int	tokens_used;

	status = 0;
	if (!args[1])
	{
		print_sorted_env(env_list);
		return (0);
	}
	
	i = 1;
	while (args[i])
	{
		tokens_used = process_export_arg(env_list, args, i, &status);
		if (tokens_used <= 0)
			i++;
		else
			i += tokens_used;
	}
	return (status);
}