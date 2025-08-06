/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_word_token_enhanced.c                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 17:19:12 by terden            #+#    #+#             */
/*   Updated: 2025/07/27 17:46:12 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	reset_token_ctx(t_token_ctx *ctx)
{
	ft_bzero(ctx, sizeof(*ctx));
}

int	is_break_char(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

static int	build_result(t_token_ctx *ctx)
{
	int	j;

	j = ctx->start;
	while (j < ctx->i)
	{
		if (ctx->input[j] == '\'' || ctx->input[j] == '\"')
			j = append_quoted(ctx, j);
		else
			j = append_unquoted(ctx, j);
		if (j < 0)
			return (-1);
	}
	return (0);
}

int	create_word_token_enhanced(t_token_ctx *ctx)
{
	init_token_ctx(ctx);
	if (!scan_input(ctx))
		return (-1);
	if (build_result(ctx) < 0)
		return (-1);
	if (!finalize_token(ctx))
		return (-1);
	return (ctx->i);
}
