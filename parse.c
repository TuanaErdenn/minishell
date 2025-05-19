#include "minishell.h"
#include <string.h>
#include <stdlib.h>

/* AST Node oluşturma */
t_astnode *create_ast_node(char *content)
{
    t_astnode *new_node = (t_astnode *)malloc(sizeof(t_astnode));
    if (!new_node)
        return NULL;
    new_node->content = strdup(content);
    new_node->left = NULL;
    new_node->right = NULL;
    return new_node;
}

/* AST Node'ları serbest bırakma */
void free_ast(t_astnode *node)
{
    if (!node)
        return;
    free_ast(node->left);
    free_ast(node->right);
    if (node->content)
        free(node->content);
    free(node);
}

/* Operatör mü? */
int is_operator(int type)
{
    return (type == 1 || type == 2 || type == 3);
}

/* Operatör önceliği */
int get_precedence(int type)
{
    if (type == 1)  // PIPE
        return 1;
    if (type == 2 || type == 3)  // REDIR_OUT, REDIR_IN
        return 2;
    return 0;
}

/* AST oluşturucu */
t_astnode *parse(t_token **tokens)
{
    t_astnode *root = NULL;
    t_astnode *current = NULL;
    int i = 0;

    while (tokens[i])
    {
        t_astnode *new_node = create_ast_node(tokens[i]->value);

        if (is_operator(tokens[i]->type))
        {
            int precedence = get_precedence(tokens[i]->type);

            if (!root || precedence >= get_precedence(root->content[0]))
            {
                new_node->left = root;
                root = new_node;
            }
            else
            {
                current = root;
                while (current->right && is_operator(current->right->content[0]))
                    current = current->right;

                new_node->left = current->right;
                current->right = new_node;
            }
        }
        else
        {
            if (!root)
            {
                root = new_node;
            }
            else
            {
                current = root;
                while (current->right)
                    current = current->right;

                current->right = new_node;
            }
        }

        i++;
    }

    return root;
}
