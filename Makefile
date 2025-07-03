NAME = minishell

CC = cc
CFLAGS = -Wall -Wextra -Werror -g #-fsanitize=address -g3

SRCS = main.c parse.c token.c builtin.c free.c env.c expander.c redir.c export.c unset.c pipe.c find.c signal.c exit.c

OBJS = $(SRCS:.c=.o)

LIBFT_DIR = ./libft
LIBFT = $(LIBFT_DIR)/libft.a

all: $(NAME)

$(NAME): $(OBJS)
	make -C $(LIBFT_DIR)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -lreadline -o $(NAME)
#-lreadline readline gibi başka kütüphaneye ait fonksiyonları tanımak için linkleme yapıyor#
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