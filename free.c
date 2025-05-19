#include "minishell.h"

/* Tek bir token'ı temizleyen fonksiyon */
void	free_token(t_token *token)
{
	if (!token)
		return ;
	if (token->value)
		free(token->value);
	free(token);
}
void free_env_list(t_env *env_list)
{
    t_env *temp;
    
    while (env_list)
    {
        temp = env_list;
        env_list = env_list->next;
        free(temp->key);
        free(temp->value);
        free(temp);
    }
}
/* Token dizisini temizleyen fonksiyon */
void	free_token_array(t_token **tokens, int count)
{
	int	i;

	if (!tokens)
		return ;
	i = 0;
	while (i < count)
	{
		if (tokens[i])
			free_token(tokens[i]);
		i++;
	}
	free(tokens);
}
