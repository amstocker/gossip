# setup libuv
cd deps/libuv/
tar xzvf v1.8.0.tar.gz
cd libuv-1.8.0/
./autogen.sh
./configure
cd ../../

# setup leveldb
cd leveldb/
tar xzvf v1.18.tar.gz
cd ../../
