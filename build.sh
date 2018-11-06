cd src
g++ -o main  -lzmq -lmsgpackc -L ../lib  -I ../include/zmq -I ../include/msgpack  -Wl,-rpath ../lib main.cpp
