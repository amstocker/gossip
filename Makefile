## Executables ##

all: gossip


## Dependencies ##

LIBUV_VER = 1.8.0
LEVELDB_VER = 1.18

LIBUV_PATH = deps/libuv/libuv-$(LIBUV_VER)
LEVELDB_PATH = deps/leveldb/leveldb-$(LEVELDB_VER)

deps/jsmn/jsmn.o:
	$(CC) -c deps/jsmn/jsmn.c -Ideps/jsmn/ -o $@

$(LIBUV_PATH)/.libs/libuv.a:
	cd deps/libuv; tar xzvf v$(LIBUV_VER).tar.gz
	cd $(LIBUV_PATH); sh autogen.sh
	cd $(LIBUV_PATH); ./configure
	$(MAKE) -C $(LIBUV_PATH)

$(LEVELDB_PATH)/libleveldb.a:
	cd deps/leveldb; tar xzvf v$(LEVELDB_VER).tar.gz
	$(MAKE) -C $(LEVELDB_PATH)


## Main ##

gossip: $(LIBUV_PATH)/.libs/libuv.a $(LEVELDB_PATH)/libleveldb.a \
				deps/jsmn/jsmn.o
	$(CC) \
		-std=c99 \
		-D_GNU_SOURCE \
		-DJSMN_PARENT_LINKS=1 \
		-I $(LIBUV_PATH)/include/ \
		-I $(LEVELDB_PATH)/include/ \
		-I deps/jsmn/ \
		$(LIBUV_PATH)/.libs/libuv.a \
		$(LEVELDB_PATH)/libleveldb.a \
		deps/jsmn/jsmn.o \
		*.c \
		-o $@ -luuid


## Clean ##

.PHONY: clean quick-clean

clean:
	$(RM) gossip

deep-clean:
	$(RM) gossip
	$(RM) deps/jsmn/jsmn.o
	$(RM) -r $(LIBUV_PATH)
	$(RM) -r $(LEVELDB_PATH)
