
#include "minishell.h"
// String array'ini temizle
void free_string_array(char **arr)
{
	if (!arr)
		return;
		
	int i = 0;
	while (arr[i])
		free(arr[i++]);
	free(arr);
}
// PATH'te komut arama fonksiyonu
char *find_exec(char *command, t_env *env_list)
{
	// 1. Komut absolute/relative path mi kontrol et
	if (ft_strchr(command, '/'))
	{
		if (access(command, F_OK) == 0 && access(command, X_OK) == 0)
			return (ft_strdup(command));
		return (NULL);
	}
	
	// 2. PATH environment variable'ını al
	char *path_env = get_env_value(env_list, "PATH");
	if (!path_env)
		return (NULL);
	
	// 3. PATH'i ':' ile böl ve her dizinde ara
	char **paths = ft_split(path_env, ':');
	if (!paths)
		return (NULL);
	
	int i = 0;
	char *full_path;
	
	while (paths[i])
	{
		// path + "/" + command birleştir
		char *temp = ft_strjoin(paths[i], "/");
		if (!temp)
		{
			free_string_array(paths);
			return (NULL);
		}
		
		full_path = ft_strjoin(temp, command);
		free(temp);
		
		if (!full_path)
		{
			free_string_array(paths);
			return (NULL);
		}
		
		// Dosya var mı ve executable mı?
		if (access(full_path, F_OK) == 0 && access(full_path, X_OK) == 0)
		{
			free_string_array(paths);
			return (full_path);
		}
		
		free(full_path);
		i++;
	}
	
	free_string_array(paths);
	return (NULL);
}// Environment'tan değer al (PATH için)

