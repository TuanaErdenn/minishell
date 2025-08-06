/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens_fill.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: terden <terden@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/27 18:07:01 by terden            #+#    #+#             */
/*   Updated: 2025/08/03 13:32:16 by terden           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	process_token_enhanced(t_token_ctx *ctx, int i)
{
	if (ctx->input[i] == '|' || ctx->input[i] == '<' || ctx->input[i] == '>')
		return (create_special_token(ctx->input, i,
				ctx->tokens, ctx->token_index));
	ctx->i = i;
	return (create_word_token_enhanced(ctx));
}

int	fill_tokens_enhanced_with_expansion(char *input, t_token **tokens,
		t_shell *shell)
{
	t_token_ctx	ctx;
	int			i;
	int			token_index;
	int			result;

	i = 0;
	token_index = 0;
	ctx.input = input;
	ctx.tokens = tokens;
	ctx.token_index = &token_index;
	ctx.shell = shell;
	while (input[i])
	{
		i = skip_spaces(input, i);
		if (!input[i])
			break ;
		result = process_token_enhanced(&ctx, i);
		if (result == -1)
			return (tokens[token_index] = NULL, 0);
		i = result;
	}
	tokens[token_index] = NULL;
	return (1);
}
