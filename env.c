#include "minishell.h"

// 25 satır sınırına uygun yeni env node oluşturma fonksiyonu
t_env *new_env_node(char *key, char *value)
{
	t_env *new;

	new = (t_env *)malloc(sizeof(t_env));
	if (!new)
		return (NULL);
	new->key = ft_strdup(key);
	if (!new->key)
	{
		free(new);
		return (NULL);
	}
	new->value = ft_strdup(value);
	if (!new->value)
	{
		free(new->key);
		free(new);
		return (NULL);
	}
	new->next = NULL;
	return (new);
}


void add_env_node(t_env **env_list, t_env *new_node)
{
	t_env *temp;

	if (!*env_list)
	{
		*env_list = new_node;
		return;
	}
	temp = *env_list;
	while (temp->next)
		temp = temp->next;
	temp->next = new_node;
}

void split_env_line(char *line, char **key, char **value)
{
	char *equal_sign;

	equal_sign = ft_strchr(line, '=');
	if (equal_sign)
	{
		// Geçici olarak ilk eşittir işaretini null karakterle değiştir
		*equal_sign = '\0';
		*key = ft_strdup(line);
		*value = ft_strdup(equal_sign + 1);
		// Orijinal string'i geri yükle
		*equal_sign = '=';
	}
	else
	{
		// Eşittir işareti yoksa, tüm string key olur, value NULL veya boş string olur
		*key = ft_strdup(line);
		*value = ft_strdup("");
	}
}

// node oluşturma ve temizleme
void create_and_add_node(t_env **env_list, char *key, char *value)
{
	t_env *new_node;

	if (key && value)
	{
		new_node = new_env_node(key, value);
		if (new_node)
			add_env_node(env_list, new_node);
	}
	if (key)
		free(key);
	if (value)
		free(value);
}

// init_env_list fonksiyonunu çift pointer olarak güncelleme
t_env *init_env_list(char **environ)
{
	t_env *env_list = NULL;
	int i = 0;
	char *key;
	char *value;

	while (environ[i])
	{
		key = NULL;
		value = NULL;
		split_env_line(environ[i], &key, &value);
		create_and_add_node(&env_list, key, value);
		i++;
	}
	return (env_list);
}


int ft_strcmp(const char *s1, const char *s2)
{
	int i = 0;

	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;

	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

// Bağlı listeden çevre değişkeninin değerini alma
// Çift pointer parametresi alacak şekilde güncellendi
char *get_env_value(t_env *env_list, char *key)
{
	t_env *temp;

	if (!env_list)
		return (NULL); // Boş string döndür

	temp = env_list;
	while (temp)
	{
		if (ft_strcmp(temp->key, key) == 0)
			return (temp->value);
		temp = temp->next;
	}
	return (NULL); // NULL değil, boş string döndürmek doğru
}