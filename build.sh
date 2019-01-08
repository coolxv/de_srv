cd src
g++ -g -std=c++11  -o gpats -lfmt -lgflags -lglog -lzmq -lmsgpackc -lmysqlclient  -lssl -lcrypto -L../lib -I../include -I../include/zmq -I../include/msgpack  -I../include/mysql/mysql -Wl,-rpath ../lib main.cpp
cd ..
mv src/gpats bin