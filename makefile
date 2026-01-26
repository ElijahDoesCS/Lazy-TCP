CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = tcp_less
OBJ_DIR = obj_files

# --- Module Variables ---
ENTRY    = main.c
V_DEVICE = v_device/v_device.c
T_POOL   = thread_pool/thread_pool.c
NETWORK  = network/network.c
ICMP     = network/icmp/icmp.c
IP       = network/ip/ip.c

# --- Combine them into one list ---
SRCS = $(ENTRY) $(V_DEVICE) $(T_POOL) $(NETWORK) $(ICMP) $(IP)

# --- Object Logic ---
# This takes the list in SRCS, strips paths, and puts them in OBJ_DIR
OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.c=.o)))

# VPATH tells 'make' where to search for the .c files listed in SRCS
VPATH = $(dir $(SRCS))

# --- Rules ---

all: $(TARGET)

$(TARGET): $(OBJ_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# The pattern rule to compile .c -> .o
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean