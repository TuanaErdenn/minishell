
#include "minishell.h"
// ft_strncpy implementasyonu
char *ft_strncpy(char *dest, const char *src, size_t n)
{
	size_t i;

	i = 0;
	while (i < n && src[i])
	{
		dest[i] = src[i];
		i++;
	}
	while (i < n)
		dest[i++] = '\0';
	return (dest);
}
/* PWD builtin - mevcut çalışma dizinini yazdırır */
int ft_pwd(void)
{
	char cwd[1024]; // Çalışma dizini için buffer

	// getcwd fonksiyonu mevcut çalışma dizinini alır
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		// Mevcut dizini yazdır ve yeni satır ekle
		printf("%s\n", cwd);
		return (0); // Başarılı
	}
	else
	{
		// Hata durumunda perror ile sistem hata mesajını yazdır
		perror("minishell: pwd");
		return (1); // Hata
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

	return (1); // Sadece -n veya -nnn... şeklinde
}

void print_with_expansion(t_env *env_list, char *str, int exit_code)
{
	int i = 0;

	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
		{
			if (str[i + 1] == '?')
			{
				printf("%d", exit_code);
				i += 2;
				continue;
			}

			i++; // $'i geç
			int var_start = i;

			// ❗ Sadece alfanümerik ve alt çizgi karakterlerini al
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
			{
				printf("$");
			}

			// 💡 Devam etmeli
			continue;
		}

		// Normal karakter
		printf("%c", str[i]);
		i++;
	}
}

/* Echo builtin - exit code döner - bağlı liste versiyonu */
int ft_echo(t_env *env_list, char **args, t_cmd cmd, int exit_code)
{
	int i = 1;
	int newline = 1;

	// -n flaglerini kontrol et
	while (args[i] && is_n_flag(args[i]))
	{
		newline = 0;
		i++;
	}

	// Argümanları yazdır
	while (args[i])
	{
		// Argümanı yazdır (değişken genişletmeli veya değil)
		if (cmd.args_quote_type[i] != 1 && ft_strchr(args[i], '$'))
		{
			print_with_expansion(env_list, args[i], exit_code);
		}
		else
		{
			// Tek tırnaklı metinlerde veya $ içermeyenlerde normal yazdır
			// Ancak çift tırnak karakterlerini kaldır
			if (cmd.args_quote_type[i] == 2)
			{
				char *p = args[i];
				while (*p)
				{
					if (*p != '"')
						printf("%c", *p);
					p++;
				}
			}
			else
			{
				printf("%s", args[i]);
			}
		}

		// Bir sonraki argüman varsa ve aynı token parçası değilse boşluk ekle
		// Bu özel bir durum kontrolü gerektiriyor,
		// mevcut verinizle tam olarak belirlemek zor
		if (args[i + 1] && cmd.args_quote_type[i] != 2)
			printf(" ");
		i++;
	}

	// Yeni satır kontrolü
	if (newline)
		printf("\n");

	return (0); // Başarılı
}
/* Env builtin - çevre değişkenlerini yazdır */
int ft_env(t_env *env_list)
{
	t_env *temp;

	temp = env_list;
	while (temp)
	{
		// KEY=VALUE formatında yazdır
		printf("%s=%s\n", temp->key, temp->value);
		temp = temp->next;
	}
	return (0);
}

/* Builtin mi kontrol et */
int is_builtin(t_cmd *cmd)
{
	if (!cmd || !cmd->args || !cmd->args[0])
		return (0);

	if (!ft_strcmp(cmd->args[0], "echo"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "pwd"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "env"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "cd"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "export"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "unset"))
		return (1);
	if (!ft_strcmp(cmd->args[0], "exit"))
		return (1);

	return (0);
}
/* Builtin çalıştır - exit code döner */
int run_builtin(t_env *env_list, t_cmd *cmd, int exit_code)
{
	if (!strcmp(cmd->args[0], "echo"))
		return (ft_echo(env_list, cmd->args, *cmd, exit_code));
	else if (!strcmp(cmd->args[0], "pwd"))
		return (ft_pwd());
	else if (!strcmp(cmd->args[0], "env"))
		return (ft_env(env_list));
	else if (!strcmp(cmd->args[0], "cd"))
		return ft_cd(env_list, cmd->args);
	return (0);
}
/* Komut çalıştır - exit code döner - bağlı liste versiyonu */
int execute_command(t_env *env_list, t_cmd *cmd, int exit_code)
{
	if (is_builtin(cmd))
		return (run_builtin(env_list, cmd, exit_code));
	else
	{
		printf("minishell: %s: command not found\n", cmd->args[0]);
		return (127); // Command not found
	}
}