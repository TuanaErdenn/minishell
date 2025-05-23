#include "minishell.h"

char *expand_string(const char *str, t_env *env, int exit_code)
{
	if (!str || str[0] != '$')
		return ft_strdup(str); // $ ile başlamıyorsa değiştirme

	if (ft_strcmp(str, "$?") == 0)
		return ft_itoa(exit_code); // özel değişken $? işleniyor

	// $VAR kısmını al (ilk karakter '$' zaten var)
	const char *key = str + 1;
	char *value = get_env_value(env, (char *)key);

    printf("get_env_value key = [%s] → [%s]\n", key, value ? value : "NULL");
	if (value)
		return ft_strdup(value);
	else
		return ft_strdup(""); // bilinmeyen değişken → boş döner
}
void expand_ast(t_ast *node, t_env *env, int exit_code)
{
	if (!node)
		return;

	if (node->type == NODE_COMMAND && node->args)
	{
		int i = 0;
		while (node->args[i])
		{
			// Debug: hangi argüman genişletilmeye çalışılıyor?
			printf("CHECK: [%s] (quote_type: %d)\n", node->args[i], node->quote_types[i]);

			// Sadece tek tırnaklı olanlar atlanır
			if (node->quote_types && node->quote_types[i] != 1)
			{
				// '$' içeriyorsa (başta ya da ortada)
				if (ft_strchr(node->args[i], '$'))
				{
					char *expanded = expand_string(node->args[i], env, exit_code);
					if (expanded)
					{
						free(node->args[i]);
						node->args[i] = expanded;
					}
				}
			}
			i++;
		}
	}

	expand_ast(node->left, env, exit_code);
	expand_ast(node->right, env, exit_code);
}
