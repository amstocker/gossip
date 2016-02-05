
.PHONY: make-dirs \
	test-json \
	clean quick-clean


## Executables ##

all: gossip


## Dependencies ##

LIBUV_VER = 1.8.0
LEVELDB_VER = 1.18

# paths
DEPS_INCLUDE = deps/include
DEPS_BUILD = deps/build
DEPS_SRC = deps/src

LIBUV_PATH = $(DEPS_SRC)/libuv/libuv-$(LIBUV_VER)
LEVELDB_PATH = $(DEPS_SRC)/leveldb/leveldb-$(LEVELDB_VER)

$(DEPS_INCLUDE):
	mkdir -p $(DEPS_INCLUDE)

$(DEPS_BUILD):
	mkdir -p $(DEPS_BUILD)

#libuv
$(DEPS_BUILD)/libuv.a: $(DEPS_BUILD) $(DEPS_INCLUDE)
	cd $(DEPS_SRC)/libuv; tar xzvf v$(LIBUV_VER).tar.gz
	cd $(LIBUV_PATH); sh autogen.sh
	cd $(LIBUV_PATH); ./configure
	$(MAKE) -C $(LIBUV_PATH)
	cp -r $(LIBUV_PATH)/include/* $(DEPS_INCLUDE)
	cp $(LIBUV_PATH)/.libs/libuv.a $(DEPS_BUILD)

# leveldb
$(DEPS_BUILD)/libleveldb.a: $(DEPS_BUILD) $(DEPS_INCLUDE)
	cd $(DEPS_SRC)/leveldb; tar xzvf v$(LEVELDB_VER).tar.gz
	$(MAKE) -C $(LEVELDB_PATH)
	cp -r $(LEVELDB_PATH)/include/* $(DEPS_INCLUDE)
	cp $(LEVELDB_PATH)/libleveldb.a $(DEPS_BUILD)


## Main ##

DEPS = \
	$(DEPS_BUILD)/libuv.a \
	$(DEPS_BUILD)/libleveldb.a \

CFLAGS = -std=c99 \
				 -Wall -Wno-unused-variable \
				 -D_GNU_SOURCE \
				 -DJSMN_STRICT=1 -DJSMN_PARENT_LINKS=1
LDFLAGS = -luuid -lpthread

gossip: $(DEPS)
	$(CC) $(CFLAGS) \
		-I. \
		thirdparty/*.c utils/*.c *.c \
		-o $@ \
		$(DEPS_BUILD)/* $(LDFLAGS)


## Tests ##

test-json: $(DEPS)
	@$(CC) $(CFLAGS) -I. -I $(DEPS_INCLUDE) $(DEPS_BUILD)/* \
		hash.c comparator.c utils.c map.c json.c thirdparty/jsmn.c \
		tests/test_json.c \
		-o __$@ $(LDFLAGS)
	@./__$@
	@$(RM) __$@


## Clean ##

clean:
	$(RM) gossip
	$(RM) __test*

deep-clean: clean
	$(RM) -r $(DEPS_INCLUDE)
	$(RM) -r $(DEPS_BUILD)
	$(RM) -r $(LIBUV_PATH)
	$(RM) -r $(LEVELDB_PATH)
