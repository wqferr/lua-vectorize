LUA_DIR=/usr/local
LUA_VERSION=5.4
LUA_LIBDIR=$(LUA_DIR)/lib/lua/$(LUA_VERSION)
C_FLAGS= -Wall -Wextra
LIBFLAG= -shared -fpic

.PHONY: clean

vectorize.so: vectorize.c
	$(CC) -o vectorize.so $(LIBFLAG) $(C_FLAGS) $(CFLAGS) vectorize.c -I$(LUA_LIBDIR) -llua

clean:
	$(RM) vectorize.so
