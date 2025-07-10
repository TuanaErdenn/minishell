/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin2.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zyilmaz <zyilmaz@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 16:38:58 by zyilmaz           #+#    #+#             */
/*   Updated: 2025/07/09 19:26:49 by zyilmaz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
char *create_env_string(t_env *current)
{
    char *temp;
    char *result;
    
    temp = ft_strjoin(current->key, "=");
    if (!temp)
        return (NULL);
    result = ft_strjoin(temp, current->value ? current->value : "");
    free(temp);
    return (result);
}

int count_env_list(t_env *env_list)
{
    int count;
    t_env *current;
    
    count = 0;
    current = env_list;
    while (current)
    {
        count++;
        current = current->next;
    }
    return (count);
}

int fill_env_array(char **envp, t_env *env_list, int count)
{
    int i;
    t_env *current;
    
    i = 0;
    current = env_list;
    while (current && i < count)
    {
        envp[i] = create_env_string(current);
        if (!envp[i])
        {
            free_env_array(envp);
            return (0);
        }
        current = current->next;
        i++;
    }
    envp[i] = NULL;
    return (1);
}

char **env_to_array(t_env *env_list)
{
    int count;
    char **envp;
    
    if (!env_list)
        return (NULL);
    count = count_env_list(env_list);
    envp = malloc(sizeof(char *) * (count + 1));
    if (!envp)
        return (NULL);
    if (!fill_env_array(envp, env_list, count))
        return (NULL);
    return (envp);
}
