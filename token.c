/* Enhanced token.c with adjacent quote handling */
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

/* Enhanced word token processing - handles adjacent quotes */
int process_word_token_enhanced(char *input, int i, int *count)
{
	while (input[i] && !is_space(input[i]) && 
		   input[i] != '|' && input[i] != '<' && input[i] != '>')
	{
		if (input[i] == '\'' || input[i] == '\"')
		{
			// Tırnak içini atla
			char quote = input[i];
			i++; // Açılış tırnağını atla
			while (input[i] && input[i] != quote)
				i++;
			if (input[i])
				i++; // Kapanış tırnağını atla
		}
		else
		{
			i++;
		}
	}

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

/* Enhanced token counting */
int count_tokens_enhanced(char *input)
{
	int count = 0;
	int i = 0;

	while (input[i])
	{
		i = skip_spaces(input, i);

		if (!input[i])
			break;

		if (input[i] == '|' || input[i] == '<' || input[i] == '>')
			i = process_special_token(input, i, &count);
		else
			i = process_word_token_enhanced(input, i, &count);
	}

	return count;
}

/* Token sayısını hesaplama */
int count_tokens(char *input)
{
	return count_tokens_enhanced(input);
}

/* Token oluştur (quote_type değeri ile) */
t_token *create_token_with_quote(char *value, t_token_type type, int quote_type)
{
	t_token *token = (t_token *)malloc(sizeof(t_token));
	if (!token)
		return NULL;

	token->value = ft_strdup(value);
	if (!token->value)
	{
		free(token);
		return NULL;
	}

	token->type = type;
	token->quote_type = quote_type;
	token->next = NULL;
	return token;
}

/* Normal create_token fonksiyonunu güncelle */
t_token *create_token(char *value, t_token_type type)
{
	return create_token_with_quote(value, type, Q_NONE);
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
		quote_type = Q_SINGLE;
	else
		quote_type = Q_DOUBLE;

	tokens[*token_index] = create_token_with_quote(value, T_WORD, quote_type);
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

/* Enhanced word token creation - handles adjacent quotes with proper quote type */
int create_word_token_enhanced(char *input, int i, t_token **tokens, int *token_index)
{
	int start = i;
	char *result = NULL;
	char *temp_result = NULL;
	int has_quotes = 0;
	int quote_type = Q_NONE;
	int single_quote_count = 0;
	int double_quote_count = 0;

	// Önce tüm token'ı tara ve quote tipini belirle
	int temp_i = i;
	while (input[temp_i] && !is_space(input[temp_i]) && 
		   input[temp_i] != '|' && input[temp_i] != '<' && input[temp_i] != '>')
	{
		if (input[temp_i] == '\'')
		{
			has_quotes = 1;
			single_quote_count++;
			// Tırnak içini atla
			temp_i++; // Açılış tırnağını atla
			while (input[temp_i] && input[temp_i] != '\'')
				temp_i++;
			if (input[temp_i])
				temp_i++; // Kapanış tırnağını atla
		}
		else if (input[temp_i] == '\"')
		{
			has_quotes = 1;
			double_quote_count++;
			// Tırnak içini atla
			temp_i++; // Açılış tırnağını atla
			while (input[temp_i] && input[temp_i] != '\"')
				temp_i++;
			if (input[temp_i])
				temp_i++; // Kapanış tırnağını atla
		}
		else
		{
			temp_i++;
		}
	}

	// Quote tipini belirle
	if (single_quote_count > 0 && double_quote_count == 0)
		quote_type = Q_SINGLE;
	else if (double_quote_count > 0 && single_quote_count == 0)
		quote_type = Q_DOUBLE;
	else if (single_quote_count > 0 || double_quote_count > 0)
		quote_type = Q_DOUBLE; // Karışık durumda çift tırnak davranışı

	// Token uzunluğunu ayarla
	i = temp_i;

	// Eğer tırnak varsa, içeriği birleştir
	if (has_quotes)
	{
		result = ft_strdup(""); // Boş string ile başla
		if (!result)
			return (-1);

		int j = start;
		while (j < i)
		{
			if (input[j] == '\'' || input[j] == '\"')
			{
				// Tırnak içeriğini al
				char quote = input[j];
				j++; // Açılış tırnağını atla
				int quote_start = j;
				while (j < i && input[j] != quote)
					j++;
				
				if (j > quote_start)
				{
					char *quoted_content = ft_substr(input, quote_start, j - quote_start);
					if (quoted_content)
					{
						temp_result = ft_strjoin(result, quoted_content);
						free(result);
						free(quoted_content);
						if (!temp_result)
							return (-1);
						result = temp_result;
					}
				}
				if (j < i && input[j] == quote)
					j++; // Kapanış tırnağını atla
			}
			else
			{
				// Normal karakter
				char temp_char[2] = {input[j], '\0'};
				temp_result = ft_strjoin(result, temp_char);
				free(result);
				if (!temp_result)
					return (-1);
				result = temp_result;
				j++;
			}
		}
	}
	else
	{
		// Tırnak yoksa normal şekilde al
		int len = i - start;
		result = ft_substr(input, start, len);
		if (!result)
			return (-1);
	}

	// Token'ı quote type ile oluştur
	tokens[*token_index] = create_token_with_quote(result, T_WORD, quote_type);
	free(result);

	if (!tokens[*token_index])
		return (-1);

	(*token_index)++;
	return i;
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

/* Enhanced token filling */
int fill_tokens_enhanced(char *input, t_token **tokens)
{
	int i = 0;
	int token_index = 0;

	while (input[i])
	{
		i = skip_spaces(input, i);
		if (!input[i])
			break;

		int result = -1;
		
		if (input[i] == '|' || input[i] == '<' || input[i] == '>')
		{
			result = create_special_token(input, i, tokens, &token_index);
		}
		else
		{
			// Enhanced word token handling
			result = create_word_token_enhanced(input, i, tokens, &token_index);
		}

		if (result == -1)
		{
			tokens[token_index] = NULL;
			return 0;
		}
		i = result;
	}

	tokens[token_index] = NULL;
	return 1;
}

/* Token'ları oluştur ve diziye ekle */
int fill_tokens(char *input, t_token **tokens)
{
	return fill_tokens_enhanced(input, tokens);
}

/* Tokenize ana fonksiyonu */
t_token **tokenize(char *input)
{
	t_token **tokens;
	int token_count;

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