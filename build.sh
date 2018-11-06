cd src
#g++ -g -std=c++11 -DZMQ_STATIC -o main  -Wl,-dn -lzmq -lmsgpackc -lmysqlcppconn-static -Wl,-dy -lssl -lcrypto -lpthread -lrt -ldl -lm -L ../lib  -I ../include/zmq -I ../include/msgpack -I ../include/mysql/ -Wl,-rpath ../lib main.cpp

g++ -g -std=c++11  -o main  -lzmq -lmsgpackc -lmysqlclient  -lssl -lcrypto -L ../lib  -I ../include/zmq -I ../include/msgpack -I ../include/mysql/mysql -Wl,-rpath ../lib main.cpp