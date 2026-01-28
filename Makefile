SERVER_NAME = server
CLIENT_NAME = client

SERVER_SRC = src/server.c
CLIENT_SRC = src/client.c

SERVER_BONUS_SRC = src/server_bonus.c
CLIENT_BONUS_SRC = src/client_bonus.c

SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
SERVER_BONUS_OBJ = $(SERVER_BONUS_SRC:.c=.o)
CLIENT_BONUS_OBJ = $(CLIENT_BONUS_SRC:.c=.o)

CC = cc
CFLAGS = -Wall -Wextra -Werror

LIBFT = ./libft/libft.a

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

all: $(LIBFT) $(SERVER_NAME) $(CLIENT_NAME)

$(LIBFT):
	@make -s -C libft

$(SERVER_NAME): $(SERVER_OBJ)
	@$(CC) $(CFLAGS) $(SERVER_OBJ) $(LIBFT) -o $(SERVER_NAME)

$(CLIENT_NAME): $(CLIENT_OBJ)
	@$(CC) $(CFLAGS) $(CLIENT_OBJ) $(LIBFT) -o $(CLIENT_NAME)

.bonus: $(LIBFT) $(SERVER_BONUS_OBJ) $(CLIENT_BONUS_OBJ)
	@$(CC) $(CFLAGS) $(SERVER_BONUS_OBJ) $(LIBFT) -o $(SERVER_NAME)
	@$(CC) $(CFLAGS) $(CLIENT_BONUS_OBJ) $(LIBFT) -o $(CLIENT_NAME)
	@touch .bonus

bonus: .bonus

clean:
	@rm -f $(SERVER_OBJ) $(CLIENT_OBJ) $(SERVER_BONUS_OBJ) $(CLIENT_BONUS_OBJ)
	@make -s -C libft clean

fclean: clean
	@rm -f $(SERVER_NAME) $(CLIENT_NAME) .bonus
	@make -s -C libft fclean

re: fclean all

.PHONY: all clean fclean re
