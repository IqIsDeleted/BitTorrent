CC = gcc

CFLAGS = -Wall -Werror -Wextra -pedantic -O2 -Wno-deprecated-declarations
LDFLAGS =
LDLIBS = -lcrypto
DB =

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
NETWORK_DIR = network
TORRENT_CREATOR_DIR = torrent_creator
CONFIG_DIR = config
SIGNALS_DIR = signals
FILE_DIR = file
COMMON_DIR = common
HASH_DIR = hash
UI_DIR = ui

NETWORK_SRC = common.c tcp_client.c tcp_server.c udp_broadcast_receiver.c udp_broadcast.c
TORRENT_CREATOR_SRC = torrent_creator.c
CONFIG_SRC = config.c
SIGNALS_SRC = signals.c
FILE_SRC = torrent_parser.c file_assembler.c file_reader.c
COMMON_SRC = epoll_utils.c network_utils.c bitfield.c path_utils.c client_list.c
HASH_SRC = hash.c table.c
UI_SRC = progress_bar.c
MAIN_SRC = seeder.c leecher.c main.c 

NETWORK_OBJS = $(addprefix $(SRC_DIR)/$(NETWORK_DIR)/, $(NETWORK_SRC:.c=.o))
TORRENT_CREATOR_OBJS = $(addprefix $(SRC_DIR)/$(TORRENT_CREATOR_DIR)/, $(TORRENT_CREATOR_SRC:.c=.o))
CONFIG_OBJS = $(addprefix $(SRC_DIR)/$(CONFIG_DIR)/, $(CONFIG_SRC:.c=.o))
SIGNALS_OBJS = $(addprefix $(SRC_DIR)/$(SIGNALS_DIR)/, $(SIGNALS_SRC:.c=.o))
FILE_OBJS = $(addprefix $(SRC_DIR)/$(FILE_DIR)/, $(FILE_SRC:.c=.o))
COMMON_OBJS = $(addprefix $(SRC_DIR)/$(COMMON_DIR)/, $(COMMON_SRC:.c=.o))
HASH_OBJS = $(addprefix $(SRC_DIR)/$(HASH_DIR)/, $(HASH_SRC:.c=.o))
UI_OBJS = $(addprefix $(SRC_DIR)/$(UI_DIR)/, $(UI_SRC:.c=.o))
MAIN_OBJS = $(addprefix $(SRC_DIR)/, $(MAIN_SRC:.c=.o))

TORRENT_CREATOR_BIN = $(BIN_DIR)/creator
MAIN_BIN = $(BIN_DIR)/main

.PHONY: all clean style deps $(BIN_DIR) $(OBJ_DIR)

all: deps $(MAIN_BIN) $(TORRENT_CREATOR_BIN)

deps: src/thirdparty/uthash.h

test: DB := -g
test: $(MAIN_BIN) $(TORRENT_CREATOR_BIN)

$(MAIN_BIN): $(MAIN_OBJS) $(CONFIG_OBJS) $(SIGNALS_OBJS) $(FILE_OBJS) $(NETWORK_OBJS) $(COMMON_OBJS) $(HASH_OBJS) $(UI_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(DB) -o $@ $(addprefix $(OBJ_DIR)/, $(notdir $^)) $(LDFLAGS) $(LDLIBS)

$(TORRENT_CREATOR_BIN): $(TORRENT_CREATOR_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(DB) -o $@ $(addprefix $(OBJ_DIR)/, $(notdir $^)) $(LDFLAGS) $(LDLIBS)

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

src/thirdparty/uthash.h:
	@mkdir -p src/thirdparty
	@if [ ! -f src/thirdparty/uthash.h ]; then \
		curl -L -o src/thirdparty/uthash.h https://github.com/troydhanson/uthash/raw/refs/heads/master/src/uthash.h; \
	fi

clean:
	rm -r $(BIN_DIR) $(OBJ_DIR)

style:
	cp linters/.clang-format .clang-format
	find . -name "*.c" -o -name "*.h" -exec clang-format -i {} \;
	@find . -name "*.c" -exec clang-format -style="{BasedOnStyle: Google, InsertBraces: true}" --verbose --Werror --ferror-limit=0 -i {} +
	@cppcheck --enable=all --suppress=missingIncludeSystem --suppress=checkersReport --suppress=unusedFunction --check-level=exhaustive -I. .
	rm .clang-format

%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(DB) $(INCLUDES) -c $< -o $(OBJ_DIR)/$(notdir $@)
