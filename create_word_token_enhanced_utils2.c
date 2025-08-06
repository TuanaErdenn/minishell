/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_word_token_enhanced_utils2.c                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 17:40:31 by terden            #+#    #+#             */
/*   Updated: 2025/07/27 17:45:30 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	set_quote_type(t_token_ctx *ctx)
{
	if (ctx->single_quote_count > 0 && ctx->double_quote_count == 0)
		ctx->quote_type = Q_SINGLE;
	else if (ctx->double_quote_count > 0 && ctx->single_quote_count == 0)
		ctx->quote_type = Q_DOUBLE;
	else if (ctx->single_quote_count > 0 || ctx->double_quote_count > 0)
		ctx->quote_type = Q_DOUBLE;
	else
		ctx->quote_type = Q_NONE;
}

static int	handle_single_double_quotes(t_token_ctx *ctx, int temp_i)
{
	if (ctx->input[temp_i] == '\'')
	{
		ctx->single_quote_count++;
		ctx->only_var_expand = 0;
		temp_i++;
		while (ctx->input[temp_i] && ctx->input[temp_i] != '\'')
			temp_i++;
		if (ctx->input[temp_i])
			temp_i++;
	}
	else if (ctx->input[temp_i] == '\"')
	{
		ctx->double_quote_count++;
		ctx->only_var_expand = 0;
		temp_i++;
		while (ctx->input[temp_i] && ctx->input[temp_i] != '\"')
			temp_i++;
		if (ctx->input[temp_i])
			temp_i++;
	}
	return (temp_i);
}

int	handle_quotes_and_vars(t_token_ctx *ctx, int temp_i)
{
	if (ctx->input[temp_i] == '\''
		|| ctx->input[temp_i] == '\"')
		return (handle_single_double_quotes(ctx, temp_i));
	else if (ctx->input[temp_i] == '$')
	{
		temp_i++;
		while (ctx->input[temp_i]
			&& (ft_isalnum(ctx->input[temp_i]) || ctx->input[temp_i] == '_'))
			temp_i++;
	}
	else
	{
		ctx->only_var_expand = 0;
		temp_i++;
	}
	return (temp_i);
}

char	*join_and_free(t_token_ctx *ctx, char *str)
{
	char	*tmp;

	tmp = ft_strjoin(ctx->result, str);
	free(ctx->result);
	free(str);
	ctx->result = tmp;
	return (ctx->result);
}

char	*expand_and_join(t_token_ctx *ctx, char *str)
{
	char	*expanded;
	char	*tmp;

	expanded = expand_string_with_vars(str, ctx->shell);
	free(str);
	if (!expanded)
		return (ctx->result);
	tmp = ft_strjoin(ctx->result, expanded);
	free(expanded);
	free(ctx->result);
	ctx->result = tmp;
	return (ctx->result);
}
