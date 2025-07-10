// exit.c
#include "minishell.h"

static int	is_numeric(const char *str)
{
	int	i = 0;

	if (!str || str[0] == '\0')
		return (0);
	if (str[0] == '+' || str[0] == '-')
		i++;
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}

static long	ft_atol(const char *str)
{
	int		i = 0;
	long	res = 0;
	int		sign = 1;

	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (ft_isdigit(str[i]))
	{
		res = res * 10 + (str[i] - '0');
		i++;
	}
	return (res * sign);
}

int	handle_exit(char *input, t_env *env_list)
{
	(void)env_list;
	if (ft_strcmp(input, "exit") == 0)
	{
		write(1, "exit\n", 5);
		free(input);
		return (1);
	}
	return (0);
}

int	builtin_exit(char **args, t_shell *shell)
{
	int	argc = 0;
	long	val;

	while (args[argc])
		argc++;

	ft_putendl_fd("exit", STDOUT_FILENO);

	if (argc == 1)
		exit(shell->exit_code);
	if (!is_numeric(args[1]))
	{
		ft_putstr_fd("minishell: exit: ", STDERR_FILENO);
		ft_putstr_fd(args[1], STDERR_FILENO);
		ft_putendl_fd(": numeric argument required", STDERR_FILENO);
		exit(2);
	}
	if (argc > 2)
	{
		ft_putendl_fd(" too many arguments", STDERR_FILENO);
		shell->exit_code = 1;
		return (1); // Shell burada çıkmamalı!
	}
	val = ft_atol(args[1]);
	exit((unsigned char)val);
}
