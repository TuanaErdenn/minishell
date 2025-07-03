#include "minishell.h"

// ft_strncpy implementasyonu
char *ft_strncpy(char *dest, const char *src, size_t n)
{
	size_t i = 0;
	while (i < n && src[i])
	{
		dest[i] = src[i];
		i++;
	}
	while (i < n)
		dest[i++] = '\0';
	return (dest);
}

/* PWD builtin - uses PWD environment variable if available (like bash) */
int ft_pwd(t_env *env_list)
{
	char *pwd_env;
	char cwd[1024];
	
	// First try to get PWD from environment (logical path)
	pwd_env = get_env_value(env_list, "PWD");
	if (pwd_env && pwd_env[0] != '\0')
	{
		printf("%s\n", pwd_env);
		return (0);
	}
	
	// Fallback to getcwd (physical path)
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("%s\n", cwd);
		return (0);
	}
	else
	{
		perror("minishell: pwd");
		return (1);
	}
}

int is_n_flag(const char *str)
{
	int i = 1;
	if (!str || str[0] != '-')
		return (0);
	while (str[i])
	{
		if (str[i] != 'n')
			return (0);
		i++;
	}
	return (1);
}

/* CD builtin */
int ft_cd(t_env *env_list, char **args)
{
	char *path;
	char current_dir[1024];
	int arg_count = 0;
	char *home;
	char *new_dir;

	if (getcwd(current_dir, sizeof(current_dir)) == NULL)
	{
		perror("minishell: cd");
		return (1);
	}
	while (args[arg_count])
		arg_count++;
	if (arg_count > 2)
	{
		ft_putstr_fd("too many arguments\n", STDERR_FILENO);
		return (1);
	}
	if (!args[1] || ft_strcmp(args[1], "~") == 0)
	{
		path = get_env_value(env_list, "HOME");
		if (!path || !*path)
		{
			ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
			return (1);
		}
	}
	else if (ft_strcmp(args[1], "-") == 0)
	{
		path = get_env_value(env_list, "OLDPWD");
		if (!path || !*path)
		{
			ft_putstr_fd("minishell: cd: OLDPWD not set\n", STDERR_FILENO);
			return (1);
		}
		printf("%s\n", path);
	}
	else if (args[1][0] == '~' && args[1][1] == '/')
	{
		home = get_env_value(env_list, "HOME");
		if (!home || !*home)
		{
			ft_putstr_fd("minishell: cd: HOME not set\n", STDERR_FILENO);
			return (1);
		}
		path = ft_strjoin(home, &args[1][1]);
		if (!path)
			return (1);
	}
	else
		path = args[1];

	if (chdir(path) != 0)
	{
		perror("minishell: cd");
		if (args[1] && args[1][0] == '~' && args[1][1] == '/')
			free(path);
		return (1);
	}
	new_dir = getcwd(NULL, 0);
	if (!new_dir)
	{
		perror("minishell: cd");
		if (args[1] && args[1][0] == '~' && args[1][1] == '/')
			free(path);
		return (1);
	}
	add_update_env(&env_list, "OLDPWD", current_dir);
	add_update_env(&env_list, "PWD", new_dir);
	free(new_dir);
	if (args[1] && args[1][0] == '~' && args[1][1] == '/')
		free(path);
	return (0);
}

void print_with_expansion(t_env *env_list, char *str, t_shell *shell)
{
	int i = 0;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
		{
			if (str[i + 1] == '?')
			{
				printf("%d", shell->exit_code);
				i += 2;
				continue;
			}
			i++;
			int var_start = i;
			while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
				i++;
			int var_len = i - var_start;
			if (var_len > 0)
			{
				char var_name[256];
				ft_strncpy(var_name, &str[var_start], var_len);
				var_name[var_len] = '\0';
				char *value = get_env_value(env_list, var_name);
				if (value)
					printf("%s", value);
			}
			else
				printf("$");
			continue;
		}
		printf("%c", str[i]);
		i++;
	}
}

int ft_echo(t_env *env_list, char **args, t_cmd cmd, t_shell *shell)
{
    int i = 1;
    int newline = 1;
    
    while (args[i] && is_n_flag(args[i]))
    {
        newline = 0;
        i++;
    }
	i = 1;
    (void)env_list;
	(void)shell;
	(void)cmd;
    while (args[i])
    {

		ft_putstr_fd(args[i], 1);

        if (args[i + 1])
            ft_putstr_fd(" ", 1); // ✅ DÜZELTME: boşluk ekle
        i++;
    }
    
    if (newline)
        ft_putstr_fd("\n", 1);
    return (0);
}

int ft_env(t_env *env_list)
{
	t_env *temp = env_list;
	while (temp)
	{
		printf("%s=%s\n", temp->key, temp->value);
		temp = temp->next;
	}
	return (0);
}

int is_builtin(t_cmd *cmd)
{
	if (!cmd || !cmd->args || !cmd->args[0])
		return (0);
	if (!ft_strcmp(cmd->args[0], "echo")) return (1);
	if (!ft_strcmp(cmd->args[0], "pwd")) return (1);
	if (!ft_strcmp(cmd->args[0], "env")) return (1);
	if (!ft_strcmp(cmd->args[0], "cd")) return (1);
	if (!ft_strcmp(cmd->args[0], "export")) return (1);
	if (!ft_strcmp(cmd->args[0], "unset")) return (1);
	if (!ft_strcmp(cmd->args[0], "exit")) return (1);
	return (0);
}

int run_builtin(t_env *env_list, t_cmd *cmd, t_shell *shell)
{
	if (!ft_strcmp(cmd->args[0], "echo"))
		return (ft_echo(env_list, cmd->args, *cmd, shell));
	else if (!ft_strcmp(cmd->args[0], "pwd"))
		return (ft_pwd(env_list));
	else if (!ft_strcmp(cmd->args[0], "env"))
		return (ft_env(env_list));
	else if (!ft_strcmp(cmd->args[0], "cd"))
		return ft_cd(env_list, cmd->args);
	else if (!ft_strcmp(cmd->args[0], "export"))
		return (execute_export(&env_list, cmd->args));
	else if (!ft_strcmp(cmd->args[0], "unset"))
		return (execute_unset(&env_list, cmd->args));
	else if (!ft_strcmp(cmd->args[0], "exit"))
		return (builtin_exit(cmd->args, shell));
	return (0);
}

// Environment listesini execve için array'e çevir
char **env_to_array(t_env *env_list)
{
    if (!env_list)
        return (NULL);
    
    // Environment sayısını hesapla
    int count = 0;
    t_env *current = env_list;
    while (current)
    {
        count++;
        current = current->next;
    }
    
    // Array oluştur
    char **envp = malloc(sizeof(char *) * (count + 1));
    if (!envp)
        return (NULL);
    
    current = env_list;
    int i = 0;
    
    while (current && i < count)
    {
        // "KEY=VALUE" formatı
        char *temp = ft_strjoin(current->key, "=");
        if (!temp)
        {
            free_env_array(envp);
            return (NULL);
        }
        
        envp[i] = ft_strjoin(temp, current->value ? current->value : "");
        free(temp);
        
        if (!envp[i])
        {
            free_env_array(envp);
            return (NULL);
        }
        
        current = current->next;
        i++;
    }
    envp[i] = NULL;
    return (envp);
}

int run_path(char *path, t_cmd *cmd, t_env *env_list, t_shell *shell)
{
    pid_t pid;
    int status;
    
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        shell->exit_code = 1;
        return (1);
    }
    
    if (pid == 0)
    {
        // === CHILD PROCESS ===
        set_signal_mode(SIGMODE_CHILD, NULL);  // ✅ SİNYAL AYARI BURADA

        // Environment array'i hazırla
        char **envp = env_to_array(env_list);
        if (!envp)
            exit(1);
        
        // Komutu çalıştır
        execve(path, cmd->args, envp);
        
        // execve başarısız olursa buraya gelir
        perror("execve");
        free_env_array(envp);
        exit(126); // execution error
    }
    else
    {
        // === PARENT PROCESS ===
        
        // Child'ın bitmesini bekle
        waitpid(pid, &status, 0);
        
        // Exit code'u ayarla
        if (WIFEXITED(status))
        {
            shell->exit_code = WEXITSTATUS(status);
            return (WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            // Signal ile sonlandırıldıysa
            shell->exit_code = 128 + WTERMSIG(status);
            return (128 + WTERMSIG(status));
        }
        else
        {
            shell->exit_code = 1;
            return (1);
        }
    }
}

/* ✅ FIXED: Proper error handling for command execution */
int execute_command(t_env *env_list, t_cmd *cmd, t_shell *shell)
{
	char *path;
	
	// 0. Boş komut kontrolü
	if (!cmd || !cmd->args || !cmd->args[0] || !*(cmd->args[0]))
	{
		// Boş komut - hiçbir şey yapma, sessizce geç
		shell->exit_code = 0;
		return (0);
	}
	
	// 1. Built-in kontrolü
	if (is_builtin(cmd))
		return (run_builtin(env_list, cmd, shell));
	
	// 2. Check if it's a path (contains '/')
	if (ft_strchr(cmd->args[0], '/'))
	{
		// It's a path - check if file exists and is executable
		if (access(cmd->args[0], F_OK) != 0)
		{
			// File doesn't exist
			fprintf(stderr, "minishell: %s: No such file or directory\n", cmd->args[0]);
			shell->exit_code = 127;
			return (127);
		}
		else if (access(cmd->args[0], X_OK) != 0)
		{
			// File exists but not executable
			fprintf(stderr, "minishell: %s: Permission denied\n", cmd->args[0]);
			shell->exit_code = 126;
			return (126);
		}
		else
		{
			// File exists and is executable
			int result = run_path(cmd->args[0], cmd, env_list, shell);
			return (result);
		}
	}
	
	// 3. Search in PATH
	path = find_exec(cmd->args[0], env_list);
	if (path)
	{
		int result = run_path(path, cmd, env_list, shell);
		free(path);
		return (result);
	}
	else
	{
		// ✅ FIXED: Print to stderr instead of stdout
		fprintf(stderr, "minishell: %s: command not found\n", cmd->args[0]);
		shell->exit_code = 127;
		return (127);
	}
}