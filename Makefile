
.PHONY: make-dirs clean quick-clean


## Executables ##

all: gossip


## Dependencies ##

LIBUV_VER = 1.8.0
LEVELDB_VER = 1.18

# paths
DEPS_INCLUDE = deps/include
DEPS_BUILD = deps/build
DEPS_SRC = deps/src

make-dirs:
	mkdir -p $(DEPS_INCLUDE)
	mkdir -p $(DEPS_BUILD)

JSMN_PATH = $(DEPS_SRC)/jsmn
LIBUV_PATH = $(DEPS_SRC)/libuv/libuv-$(LIBUV_VER)
LEVELDB_PATH = $(DEPS_SRC)/leveldb/leveldb-$(LEVELDB_VER)

# jsmn
$(JSMN_PATH)/jsmn.o: make-dirs
	$(CC) -c $(JSMN_PATH)/jsmn.c -I $(JSMN_PATH) -o $@
	cp $(JSMN_PATH)/jsmn.h $(DEPS_INCLUDE)
	cp $@ $(DEPS_BUILD)

#libuv
$(LIBUV_PATH)/.libs/libuv.a: make-dirs
	cd $(DEPS_SRC)/libuv; tar xzvf v$(LIBUV_VER).tar.gz
	cd $(LIBUV_PATH); sh autogen.sh
	cd $(LIBUV_PATH); ./configure
	$(MAKE) -C $(LIBUV_PATH)
	cp -r $(LIBUV_PATH)/include/* $(DEPS_INCLUDE)
	cp $@ $(DEPS_BUILD)

# leveldb
$(LEVELDB_PATH)/libleveldb.a: make-dirs
	cd $(DEPS_SRC)/leveldb; tar xzvf v$(LEVELDB_VER).tar.gz
	$(MAKE) -C $(LEVELDB_PATH)
	cp -r $(LEVELDB_PATH)/include/* $(DEPS_INCLUDE)
	cp $@ $(DEPS_BUILD)


## Main ##

DEPS = make-dirs \
	$(LIBUV_PATH)/.libs/libuv.a \
	$(LEVELDB_PATH)/libleveldb.a \
	$(JSMN_PATH)/jsmn.o

CFLAGS = -std=c99 -Wall -D_GNU_SOURCE -DJSMN_PARENT_LINKS=1
LDFLAGS = -luuid

gossip: $(DEPS)
	$(CC) $(CFLAGS) -I $(DEPS_INCLUDE) $(DEPS_BUILD)/* *.c -o $@ $(LDFLAGS)


## Clean ##

clean:
	$(RM) gossip

deep-clean:
	$(RM) gossip
	$(RM) -r $(DEPS_INCLUDE)
	$(RM) -r $(DEPS_BUILD)
	$(RM) $(JSMN_PATH)/jsmn.o
	$(RM) -r $(LIBUV_PATH)
	$(RM) -r $(LEVELDB_PATH)
