#include "minishell.h"

void	free_key_value(char *key, char *value)
{
	if (key)
		free(key);
	if (value)
		free(value);
}

// key value formatını parse et
int	parse_export_arg(char *arg, char **key, char **value)
{
	char	*equal;

	if (!arg || !key || !value)
		return (1);

	equal = ft_strchr(arg, '=');
	if (!equal)
	{
		// Eşittir işareti yoksa, tüm argüman key olur
		*key = ft_strdup(arg);
		*value = NULL;
		if (!*key)
			return (1);
		return (0);
	}
	// Key kısmını ayır (eşittir işaretine kadar olan kısım)
	*key = ft_substr(arg, 0, equal - arg);
	if (!*key)
		return (1);

	// Value kısmını ayır (eşittir işaretinden sonraki kısım)
	*value = ft_strdup(equal + 1);
	if (!*value)
	{
		free(*key);
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
		ft_putstr_fd("declare -x ", 1);
		ft_putstr_fd(sorted_array[i]->key, 1);
		if (sorted_array[i]->value)
		{
			ft_putstr_fd("=\"", 1);
			ft_putstr_fd(sorted_array[i]->value, 1);
			ft_putstr_fd("\"", 1);
		}
		ft_putstr_fd("\n", 1);
	}
	free(sorted_array);
}

int	process_export_arg(t_env **env_list, char **args, int i, int *status)
{
	char	*key;
	char	*value;

	key = NULL;
	value = NULL;
	if (parse_export_arg(args[i], &key, &value) != 0)
	{
		ft_putstr_fd("minishell: export: error parsing argument\n", 2);
		*status = 1;
		return (0);
	}
	if (!is_valid_identifier(key))
	{
		ft_putstr_fd("minishell: export: `", 2);
		ft_putstr_fd(args[i], 2);
		ft_putstr_fd("': not a valid identifier\n", 2);
		*status = 1;
	}
	else if (add_update_env(env_list, key, value) != 0)
		*status = 1;
	free_key_value(key, value);
	return (1);
}

int	execute_export(t_env **env_list, char **args)
{
	int	i;
	int	status;

	status = 0;
	if (!args[1])
	{
		print_sorted_env(env_list);
		return (0);
	}
	i = 1;
	while (args[i])
	{
		if (!process_export_arg(env_list, args, i, &status))
			i++;
		else
			i++;
	}
	return (status);
}