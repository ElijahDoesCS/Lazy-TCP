CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = tcp_less
OBJ_DIR = obj_files

ENTRY    = main.c
TUN 	 = tun/tun_device.c
# T_POOL   = thread_pool/thread_pool.c
NETWORK  = network/network.c
ICMP     = network/icmp/icmp.c
IP       = network/ip.c
# TCP	   = network/tcp/tcp.c
# HTTP 	   = network/http/http.c
DEBUG 	 = debug/debug.c

SRCS = $(ENTRY) $(TUN) $(NETWORK) $(ICMP) $(DEBUG) $(IP)

OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.c=.o)))

# VPATH tells 'make' where to search for the .c files listed in SRCS
VPATH = $(dir $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJ_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# The pattern rule to compile .c -> .o
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

cap:
	sudo setcap cap_net_admin=+ep ./$(TARGET)
	@echo "Capabilities granted. Run ./$(TARGET) as normal user."

uncap:
	sudo setcap -r ./$(TARGET) 2>/dev/null || true
	@echo "Capabilities removed."

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean