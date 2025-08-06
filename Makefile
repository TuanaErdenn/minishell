NAME = minishell

CC = cc
CFLAGS = -Wall -Wextra -Werror -g

SRCS = main.c parse.c token.c builtin.c free.c \
		env.c env_utils.c expander.c expander_utils.c \
		redir.c export.c merge_export_tokens.c parse_export_arg.c \
		print_sorted_env.c process_export_arg.c create_env_node.c \
		unset.c pipe.c find.c signal.c exit.c builtin2.c builtin3.c \
		builtin4.c builtin5.c signal_handle.c main_utils.c \
		create_word_token_enhanced.c create_word_token_enhanced_utils.c \
		create_word_token_enhanced_utils2.c \
		token_utils1.c token_utils2.c token_utils3.c \
		token_utils_special.c tokens_fill.c \
		pipe_1.c pipe_2.c pipe_3.c pipe_4.c pipe_child.c \
		parse_execute_ast.c parse_command.c parse_command_utils.c \
		free_ast.c create_ast_node.c pipe_child_exec.c \
		pipe_5.c redir2.c redir3.c redir4.c redir5.c redir6.c redir7.c redir8.c



OBJS = $(SRCS:.c=.o)

LIBFT_DIR = ./libft
LIBFT = $(LIBFT_DIR)/libft.a

all: $(NAME)

$(NAME): $(OBJS)
	make -C $(LIBFT_DIR)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -lreadline -o $(NAME)
clean:
	make -C $(LIBFT_DIR) clean
	rm -f $(OBJS)

fclean: clean
	make -C $(LIBFT_DIR) fclean
	rm -f $(NAME)

re: fclean all

leaks:
	@echo "Running walgrind..."
	@valgrind --leak-check=full			\
			--show-leak-kinds=all		\
			--track-origins=yes			\
			--track-fds=yes				\
			--verbose 					\
			--suppressions=valgrind.supp	\
			./$(NAME)

.PHONY: all clean fclean re