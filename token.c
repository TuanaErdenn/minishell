#include "minishell.h"

/* Token temizleme fonksiyonu */
void freetokens(t_token **tokens)
{
	int i = 0;
	if (!tokens)
		return;

	while (tokens[i])
	{
		if (tokens[i]->value)
			free(tokens[i]->value);
		free(tokens[i]);
		i++;
	}
	free(tokens);
}

/* Bir karakterin boşluk olup olmadığını kontrol etme */
int is_space(char c)
{
	return (c == ' ' || c == '\t' || c == '\n' ||
			c == '\r' || c == '\f' || c == '\v');
}

/* Boşlukları atla */
int skip_spaces(char *input, int i)
{
	while (input[i] && is_space(input[i]))
	{
		i++;
	}
	return i;
}

/* Tırnaklı token işleme */
int process_quote_token(char *input, int i, int *count)
{
	char quote = input[i];
	i++; // Tırnağı atla

	while (input[i] && input[i] != quote)
		i++;

	if (input[i])
		i++; // Kapanış tırnağını atla

	(*count)++; // Token sayacını artır
	return i;
}

/* Özel karakter token işleme */
int process_special_token(char *input, int i, int *count)
{
	if (input[i] == '<' && input[i + 1] == '<')
		i += 2; // Heredoc
	else if (input[i] == '>' && input[i + 1] == '>')
		i += 2;				  // Append
	else
		i++; // Diğer tek karakterli operatörler

	(*count)++; // Token sayacını artır
	return i;
}

/* Normal kelime token işleme */
int process_word_token(char *input, int i, int *count)
{
	while (input[i] &&
		   !is_space(input[i]) &&
		   input[i] != '|' &&
		   input[i] != '<' && input[i] != '>' &&
		   input[i] != '\'' && input[i] != '\"')
	{
		i++;
	}

	(*count)++; // Token sayacını artır
	return i;
}

/* Token sayısını hesaplama */
int count_tokens(char *input)
{
	int count = 0;
	int i = 0;

	while (input[i])
	{
		i = skip_spaces(input, i);

		if (!input[i])
			break;

		if (input[i] == '\'' || input[i] == '\"')
			i = process_quote_token(input, i, &count);
		else if (input[i] == '|' || input[i] == '<' || input[i] == '>' || input[i] == '$')
			i = process_special_token(input, i, &count);
		else
			i = process_word_token(input, i, &count);
	}

	return count;
}

/* Token oluştur (quote_type değeri ile) */
t_token *create_token_with_quote(char *value, t_token_type type, int quote_type)
{
	t_token *token = (t_token *)malloc(sizeof(t_token));
	if (!token)
		return NULL;

	if (value)
		token->value = ft_strdup(value);
	else
		token->value = ft_strdup(""); // Boş string, NULL değil

	if (!token->value)
	{
		free(token);
		return NULL;
	}

	token->type = type;
	token->quote_type = quote_type;
	return token;
}

/* Normal create_token fonksiyonunu güncelle */
t_token *create_token(char *value, t_token_type type)
{
	return create_token_with_quote(value, type, 0); // Varsayılan quote_type: 0 (no quote)
}

/* Tırnaklı token oluştur */
int create_quote_token(char *input, int i, t_token **tokens, int *token_index)
{
	char quote = input[i];
	int quote_type;
	int start = i + 1; // Başlangıç tırnağını atla
	i++;			   // İlk tırnağı atla

	while (input[i] && input[i] != quote)
		i++;

	if (!input[i]) // Eşleşen tırnak bulunamadı
		return (-1);

	int len = i - start;
	char *value = ft_substr(input, start, len); // Tırnaklar olmadan içeriği al
	if (!value)
		return (-1);

	// Tırnak tipini belirle (1: tek tırnak, 2: çift tırnak)
	if (quote == '\'')
		quote_type = 1;
	else
		quote_type = 2;

	tokens[*token_index] = create_token_with_quote(value, T_QUOTE, quote_type);
	free(value);

	if (!tokens[*token_index])
		return -1;

	(*token_index)++;
	i++; // Kapanış tırnağını atla
	return (i);
}

/* Özel karakter token oluştur */
int create_special_token(char *input, int i, t_token **tokens, int *token_index)
{
	t_token_type type = T_WORD;
	int start = i;
	int len;
	char *value;

	if (input[i] == '|')
	{
		type = T_PIPE;
		i++;
	}
	else if (input[i] == '<')
	{
		if (input[i + 1] == '<')
		{
			type = T_HEREDOC;
			i += 2;
		}
		else
		{
			type = T_INPUT;
			i++;
		}
	}
	else if (input[i] == '>')
	{
		if (input[i + 1] == '>')
		{
			type = T_APPEND;
			i += 2;
		}
		else
		{
			type = T_OUTPUT;
			i++;
		}
	}

	len = i - start;
	value = ft_substr(input, start, len);
	if (!value)
		return (-1);

	tokens[*token_index] = create_token(value, type); // quote_type = 0 (no quote)
	free(value);

	if (!tokens[*token_index])
		return (-1);

	(*token_index)++;
	return (i);
}

/* Normal kelime token oluştur */
int create_word_token(char *input, int i, t_token **tokens, int *token_index)
{
	int start = i;

	while (input[i] &&
		   !is_space(input[i]) && input[i] != '|' 
		   && input[i] != '<' && input[i] != '>' &&
		   input[i] != '\'' && input[i] != '\"')
	{
		i++;
	}

	int len = i - start;
	char *value = ft_substr(input, start, len);
	if (!value)
		return -1;

	tokens[*token_index] = create_token(value, T_WORD); // quote_type = 0 (no quote)
	free(value);

	if (!tokens[*token_index])
		return -1;

	(*token_index)++;
	return i;
}

/* Token'ları oluştur ve diziye ekle */
int fill_tokens(char *input, t_token **tokens)
{
	int i = 0;
	int token_index = 0;

	while (input[i])
	{
		i = skip_spaces(input, i);

		if (!input[i])
			break;

		if (input[i] == '\'' || input[i] == '\"')
			i = create_quote_token(input, i, tokens, &token_index);
		else if (input[i] == '|' || input[i] == '<' || input[i] == '>')
			i = create_special_token(input, i, tokens, &token_index);
		else
			i = create_word_token(input, i, tokens, &token_index);

		if (i == -1)
			return 0;
	}

	return 1;
}

/* Tokenize ana fonksiyonu */
t_token **tokenize(char *input)
{
	t_token **tokens = NULL;
	int token_count = 0;

	token_count = count_tokens(input);

	tokens = (t_token **)malloc(sizeof(t_token *) * (token_count + 1));
	if (!tokens)
		return NULL;

	if (!fill_tokens(input, tokens))
	{
		freetokens(tokens);
		return NULL;
	}

	tokens[token_count] = NULL;
	return tokens;
}
