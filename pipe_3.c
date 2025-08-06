/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_3.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 13:09:22 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/08/02 16:58:24 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	print_err(char *cmd, char *msg, int code, t_shell *sh)
{
	write(STDERR_FILENO, "minishell: ", 11);
	write(STDERR_FILENO, cmd, ft_strlen(cmd));
	write(STDERR_FILENO, msg, ft_strlen(msg));
	sh->exit_code = code;
	return (0);
}

char	*get_executable_path(char *cmd, t_env *env, t_shell *sh)
{
	struct stat	st;
	char		*path;

	if (ft_strchr(cmd, '/') || !ft_strcmp(cmd, ".") || !ft_strcmp(cmd, ".."))
	{
		if (check_path_validity(cmd, sh) != 0)
			return (NULL);
		if (stat(cmd, &st) == -1)
			return (print_err(cmd, ": No such file or directory\n", 127, sh),
				NULL);
		if (S_ISDIR(st.st_mode))
			return (print_err(cmd, ": is a directory\n", 126, sh), NULL);
		if (access(cmd, X_OK) == -1)
			return (print_err(cmd, ": Permission denied\n", 126, sh), NULL);
		return (cmd);
	}
	path = find_exec(cmd, env);
	if (!path)
		return (print_err(cmd, ": command not found\n", 127, sh), NULL);
	return (path);
}

void	execute_c_process(char *path, t_cmd *cmd, t_env *env_list,
	t_shell *shell)
{
	char	**envp;	

	set_signal_mode(SIGMODE_CHILD, NULL);
	envp = env_to_array(env_list);
	if (!envp)
		exit(1);
	execve(path, cmd->args, envp);
	perror("execve");
	free_env_list(env_list);
	free_ast(shell->ast);
	free_env_array(envp);
	free_heredoc(shell);
	free(cmd->args_quote_type);
	exit(126);
}

int	execute_command_common_child(t_env *env_list, t_cmd *cmd,
		t_shell *shell, t_ast *root_ast)
{
	validate_child_command(cmd, shell, env_list, root_ast);
	if (is_builtin(cmd))
		execute_child_builtin(env_list, cmd, shell, root_ast);
	else
		execute_child_external(env_list, cmd, shell, root_ast);
	return (0);
}

int	execute_command_common(t_env *env_list, t_cmd *cmd, t_shell *shell)
{
	int		result;	

	result = 0;
	if (!cmd || !cmd->args || !cmd->args[0])
		return (0);
	if (cmd->args[0][0] == '\0')
	{
		ft_putstr_fd("minishell: : command not found2\n", 2);
		return (127);
	}
	if (is_builtin(cmd))
	{
		result = run_builtin(env_list, cmd, shell);
		return (result);
	}
	else
	{
		result = execute_external_command(cmd, shell);
		return (result);
	}
	return (0);
}
