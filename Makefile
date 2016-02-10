
.PHONY: test debug


## Executables ##

all: gossip-server

LIBUV_VER = 1.8.0
LEVELDB_VER = 1.18
LIBUUID_VER = 1.0.3

# paths
DEPS_INCLUDE = deps/include
DEPS_BUILD = deps/build
DEPS_SRC = deps/src

LIBUV_PATH = $(DEPS_SRC)/libuv/libuv-$(LIBUV_VER)
LEVELDB_PATH = $(DEPS_SRC)/leveldb/leveldb-$(LEVELDB_VER)
LIBUUID_PATH = $(DEPS_SRC)/libuuid/libuuid-$(LIBUUID_VER)


DEPS = \
	$(DEPS_BUILD)/libuv.a \
	$(DEPS_BUILD)/libleveldb.a \
	$(DEPS_BUILD)/libuuid.a

SRC = $(wildcard src/*.c) \
	$(wildcard src/thirdparty/*.c) \
	$(wildcard src/utils/*.c)

CFLAGS = -std=c99 \
	-O2 \
	-Wall -Wno-unused-variable \
	-D_GNU_SOURCE \
	-DJSMN_STRICT=1 -DJSMN_PARENT_LINKS=1 \

INCLUDE = -I. -I./src

LDFLAGS = -lpthread

CC_SERVER = $(CC) $(CFLAGS) $(INCLUDE) $(SRC) src/bin/gossip-server.c -o gossip-server $(DEPS_BUILD)/* $(LDFLAGS)


gossip-server: $(DEPS)
	$(CC_SERVER)

debug: $(DEPS)
	$(CC_SERVER) -DDEBUG


## Dependencies ##

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

# libuuid
$(DEPS_BUILD)/libuuid.a: $(DEPS_BUILD) $(DEPS_INCLUDE)
	cd $(DEPS_SRC)/libuuid; tar xzvf v$(LIBUUID_VER).tar.gz
	cd $(LIBUUID_PATH); ./configure
	$(MAKE) -C $(LIBUUID_PATH)
	mkdir -p $(DEPS_INCLUDE)/uuid
	cp $(LIBUUID_PATH)/*.h $(DEPS_INCLUDE)/uuid
	cp $(LIBUUID_PATH)/.libs/libuuid.a $(DEPS_BUILD)


## Tests ##

test: test-json-parse \
	test-json-build \
	test-api-send \
	test-message-event

test-json-parse: debug
	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) tests/test_json_parse.c \
		-o __$@ $(DEPS_BUILD)/* $(LDFLAGS)
	./__$@
	$(RM) __$@

test-json-build: debug
	$(CC) $(CFLAGS) $(INCLUDE) $(SRC) tests/test_json_build.c \
		-o __$@ $(DEPS_BUILD)/* $(LDFLAGS)
	./__$@
	$(RM) __$@

test-message-event: debug
	@echo "[MAKE] starting test daemon ..."
	@-sh tests/test_daemon_start.sh
	@-$(CC) $(CFLAGS) $(INCLUDE) $(SRC) tests/test_send.c tests/test_message_event.c \
		-o __$@ $(DEPS_BUILD)/* $(LDFLAGS)
	@-./__$@
	@-$(RM) __$@
	@echo "[MAKE] stopping test daemon ..."
	@-sh tests/test_daemon_stop.sh

test-api-send: debug
	@echo "[MAKE] starting test daemon ..."
	@-sh tests/test_daemon_start.sh
	@-./tests/test_api_send.py
	@echo "[MAKE] stopping test daemon ..."
	@-sh tests/test_daemon_stop.sh


## Clean ##

clean:
	$(RM) gossip-*
	$(RM) gossip.sock
	$(RM) __test*

deep-clean: clean
	$(RM) -r $(DEPS_INCLUDE)
	$(RM) -r $(DEPS_BUILD)
	$(RM) -r $(LIBUV_PATH)
	$(RM) -r $(LEVELDB_PATH)
	$(RM) -r $(LIBUUID_PATH)
