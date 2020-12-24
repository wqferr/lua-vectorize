LUA_DIR=/usr/local
LUA_VERSION=5.4
LUA_LIBDIR=$(LUA_DIR)/lib/lua/$(LUA_VERSION)
LIBFLAG= -shared -fpic

.PHONY: clean

vectorize.so: vectorize.c
	$(CC) -o vectorize.so $(LIBFLAG) $(CFLAGS) vectorize.c -I$(LUA_LIBDIR) -llua

clean:
	$(RM) vectorize.so
