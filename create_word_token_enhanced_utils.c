/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_word_token_enhanced_utils.c                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 17:20:12 by terden            #+#    #+#             */
/*   Updated: 2025/07/27 17:22:10 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	init_token_ctx(t_token_ctx *ctx)
{
	ctx->start = ctx->i;
	ctx->result = ft_strdup("");
	ctx->quote_type = Q_NONE;
	ctx->single_quote_count = 0;
	ctx->double_quote_count = 0;
	ctx->only_var_expand = 1;
	ctx->should_expand = 1;
	if (*ctx->token_index > 0 && ctx->tokens[*ctx->token_index - 1]
		&& ctx->tokens[*ctx->token_index - 1]->type == T_HEREDOC)
		ctx->should_expand = 0;
}

int	scan_input(t_token_ctx *ctx)
{
	int	temp_i;

	temp_i = ctx->i;
	while (ctx->input[temp_i] && !is_space(ctx->input[temp_i]))
	{
		if (is_break_char(ctx->input[temp_i]))
			break ;
		temp_i = handle_quotes_and_vars(ctx, temp_i);
	}
	set_quote_type(ctx);
	ctx->i = temp_i;
	return (1);
}

int	append_quoted(t_token_ctx *ctx, int j)
{
	int		quote_start;
	char	*quoted_content;

	quote_start = ++j;
	while (j < ctx->i && ctx->input[j] != ctx->input[quote_start - 1])
		j++;
	quoted_content = ft_substr(ctx->input, quote_start, j - quote_start);
	if (!quoted_content)
		return (-1);
	if (ctx->input[quote_start - 1] != '\'' && ctx->should_expand
		&& ft_strchr(quoted_content, '$'))
		quoted_content = expand_and_join(ctx, quoted_content);
	else
		quoted_content = join_and_free(ctx, quoted_content);
	if (!quoted_content)
		return (-1);
	return (j + (j < ctx->i));
}

int	append_unquoted(t_token_ctx *ctx, int j)
{
	int		start;
	char	*unquoted_content;

	start = j;
	while (j < ctx->i && ctx->input[j] != '\'' && ctx->input[j] != '\"')
		j++;
	unquoted_content = ft_substr(ctx->input, start, j - start);
	if (!unquoted_content)
		return (-1);
	if (ctx->should_expand && ft_strchr(unquoted_content, '$'))
		unquoted_content = expand_and_join(ctx, unquoted_content);
	else
		unquoted_content = join_and_free(ctx, unquoted_content);
	if (!unquoted_content)
		return (-1);
	return (j);
}

int	finalize_token(t_token_ctx *ctx)
{
	if (ctx->only_var_expand && ft_strlen(ctx->result) == 0)
	{
		free(ctx->result);
		return (1);
	}
	ctx->tokens[*ctx->token_index] = create_token_with_expansion(
			ctx->result, T_WORD, ctx->quote_type, 1);
	if (!ctx->tokens[*ctx->token_index])
	{
		free(ctx->result);
		return (0);
	}
	(*ctx->token_index)++;
	free(ctx->result);
	return (1);
}
