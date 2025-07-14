#include "minishell.h"



/* ✅ FIXED: Args array expansion with empty argument removal */
char **expand_args(char **args, t_quote_type *quote_types, t_env *env_list, t_shell *shell)
{
	int i, count;
	int needs_expansion = 0;
	char **new_args;
	char **temp_args;
	int new_count = 0;

	if (!args)
		return (NULL);

	// Argüman sayısını hesapla
	count = 0;
	while (args[count])
		count++;

	// Gerçekten expansion gerekip gerekmediğini kontrol et
	i = 0;
	while (i < count && args[i])
	{
		// Tek tırnak DEĞİLSE ve içinde $ varsa expansion gerekir
		if ((!quote_types || quote_types[i] != Q_SINGLE) &&
			ft_strchr(args[i], '$'))
		{
			needs_expansion = 1;
			break;
		}
		i++;
	}

	// Expansion gerekmiyorsa NULL döndür (orijinal args'lar korunur)
	if (!needs_expansion)
		return (NULL);

	// Geçici array oluştur (expansion'dan sonra bazı argümanlar kaldırılabilir)
	temp_args = malloc(sizeof(char *) * (count + 1));
	if (!temp_args)
		return (NULL);

	// Her argümanı işle
	i = 0;
	while (i < count && args[i])
	{
		char *expanded = NULL;

		if (quote_types && quote_types[i] == Q_SINGLE)
		{
			// Tek tırnak: literal kopyala, expansion yapma
			expanded = ft_strdup(args[i]);
		}
		else if (ft_strchr(args[i], '$'))
		{
			// $ var ve tek tırnak değil: expansion yap
			expanded = expand_string_with_vars(args[i], env_list, shell);
		}
		else
		{
			// $ yok: direkt kopyala
			expanded = ft_strdup(args[i]);
		}

		if (!expanded)
		{
			// Hata durumunda temizlik
			while (--new_count >= 0)
				free(temp_args[new_count]);
			free(temp_args);
			return (NULL);
		}

		// ✅ CRITICAL: Check if this argument should be removed
		if (should_remove_arg(args[i], expanded, quote_types ? quote_types[i] : Q_NONE))
		{
			// This argument expanded to empty and should be removed
			free(expanded);
		}
		else
		{
			// Keep this argument
			temp_args[new_count] = expanded;
			new_count++;
		}

		i++;
	}
	// ✅ EDGE CASE: If all arguments were removed, return empty array with just NULL
	if (new_count == 0)
	{
		temp_args[0] = NULL;
		return temp_args;
	}
	// Resize array to actual size
	new_args = malloc(sizeof(char *) * (new_count + 1));
	if (!new_args)
	{
		while (--new_count >= 0)
			free(temp_args[new_count]);
		free(temp_args);
		return (NULL);
	}
	// Copy non-empty arguments
	for (i = 0; i < new_count; i++)
	{
		new_args[i] = temp_args[i];
	}
	new_args[new_count] = NULL;

	free(temp_args);
	return (new_args);
}

/* ✅ UPDATED: AST expansion with proper argument filtering */
void expand_ast(t_ast *node, t_env *env_list, t_shell *shell)
{
	if (!node)
		return;

	// Command node'ları için args expansion
	if (node->type == NODE_COMMAND && node->args)
	{
		char **old_args = node->args;
		t_quote_type *old_quotes = node->quote_types;
		char **expanded = expand_args(node->args, node->quote_types, env_list, shell);

		if (expanded != NULL)
		{
			// Expansion yapıldı, eski args'ları değiştir
			node->args = expanded;

			// ✅ CRITICAL: Update quote_types array to match new args
			if (old_quotes && expanded && expanded[0])
			{
				// Count new args
				int new_count = 0;
				while (expanded[new_count])
					new_count++;

				// For simplicity, set all remaining args to Q_NONE
				// (proper implementation would track which quotes remain)
				t_quote_type *new_quotes = malloc(sizeof(t_quote_type) * new_count);
				if (new_quotes)
				{
					for (int i = 0; i < new_count; i++)
						new_quotes[i] = Q_NONE; // Reset quote types after expansion

					free(old_quotes);
					node->quote_types = new_quotes;
				}
			}
			else if (old_quotes)
			{
				free(old_quotes);
				node->quote_types = NULL;
			}

			free_str_array(old_args);
		}
		// expanded == NULL: expansion gerekmedi, orijinal args'lar korunur
	}

	// Redirection node'ları için file expansion
	if (node->type == NODE_REDIR && node->file && node->file_quote != Q_SINGLE)
	{
		// Sadece $ içeriyorsa expand et
		if (ft_strchr(node->file, '$'))
		{
			char *old_file = node->file;
			char *expanded_file = expand_string_with_vars(node->file, env_list, shell);
			if (expanded_file)
			{
				node->file = expanded_file;
				free(old_file);
			}
		}
	}
	// Alt düğümleri özyinelemeli olarak expand et
	expand_ast(node->left, env_list, shell);
	expand_ast(node->right, env_list, shell);
}