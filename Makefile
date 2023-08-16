CC = cc
CFLAGS = -Wall -g
LDFLAGS = -L -lrpc
RPC_SYSTEM_A = rpc.a
CLIENT = rpc-client
SERVER = rpc-server
SRC = server.c client.c rpc.c rpc_io_helper.c array.c rpc_func_manager.c rpc_safety.c rpc_server_helper.c rpc_client_helper.c
OBJ = $(SRC:.c=.o)

.PHONY: format all

all: $(RPC_SYSTEM_A) $(SERVER) $(CLIENT)

$(RPC_SYSTEM_A): rpc.o rpc_io_helper.o array.o rpc_safety.o rpc_func_manager.o rpc_server_helper.o rpc_client_helper.o
	ar rcs $@ $^

$(SERVER): server.o $(RPC_SYSTEM_A)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT): client.o $(RPC_SYSTEM_A)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $< $(LDFLAGS)

server.o: server.c rpc.h

client.o: client.c rpc.h

rpc.o: rpc_io_helper.h array.h rpc_safety.h rpc_func_manager.h rpc_server_helper.h rpc_client_helper.h

rpc_io_helper.o: rpc_safety.h

array.o: rpc_safety.h

rpc_server_helper.o: rpc_safety.h

rpc_client_helper.o: rpc_safety.h

rpc_func_manager.o: rpc.h array.h rpc_safety.h

rpc_safety.o: rpc.h

.PHONY: clean

clean:
	rm -f *.o *.a $(SERVER) $(CLIENT)

format:
	clang-format -style=file -i *.c *.h
