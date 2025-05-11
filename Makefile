NAME = minishell

CC = cc
CFLAGS = -Wall -Wextra -Werror -g

SRCS = minishell.c signal_handler.c utils.c lexer.c token.c

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
			--show-leak-kinds=all				\
			--track-origins=yes			\
			--track-fds=yes				\
			--verbose 					\
			--suppressions=valgrind.supp	\
			./$(NAME)

.PHONY: all clean fclean re