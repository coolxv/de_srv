#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <zmq.hpp>
#include <msgpack.hpp>
#include <mysql.h>
#include "message.h"

using namespace std;

template <typename T>
static void send_data(zmq::socket_t& socket, const string& tag, const T& data)
{
	msgpack::sbuffer packed;
	msgpack::pack(&packed, data);
	//tag
	zmq::message_t tag_msg(tag.size());
	std::memcpy(tag_msg.data(), tag.data(), tag.size());
	socket.send(tag_msg, ZMQ_SNDMORE);
	//data
	zmq::message_t body_msg(packed.size());
	std::memcpy(body_msg.data(), packed.data(), packed.size());
	socket.send(body_msg);
}

template <typename T>
static void recv_data(zmq::socket_t& socket, string& tag, T& data)
{
	zmq::message_t tag_msg, body_msg;
	socket.recv(&tag_msg);
	socket.recv(&body_msg);
	//tag
	string tag_r(static_cast<const char*>(tag_msg.data()), tag_msg.size());
	tag = tag_r;
	//data
	msgpack::unpacked unpacked_body = msgpack::unpack(static_cast<const char*>(body_msg.data()), body_msg.size());
	msgpack::object deserialized = unpacked_body.get();
	deserialized.convert(data);
}

void  init_db()
{
    MYSQL mysql;
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql, "localhost", "root", "Coolxv1818581", NULL, 3306, NULL, 0))
    {
        mysql_close(&mysql);
        cout << mysql_error(&mysql) << endl;
        return;
    }

    mysql_set_character_set(&mysql, "utf8");
    char value = 1;
    mysql_options(&mysql, MYSQL_OPT_RECONNECT, &value);

    mysql_autocommit(&mysql, 1);
    
    string sql = "select * from user";
    if(0 == mysql_select_db(&mysql, "test"))
    {
        mysql_real_query(&mysql, sql.c_str(), sql.size());
    }
    //mysql_error(&mysql);

    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL_FIELD *fields;

    my_ulonglong num_rows;
    unsigned int num_fields;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            fields = mysql_fetch_field(res);
            while((row = mysql_fetch_row(res)))
            {
                for(unsigned int i = 0; i < num_fields; i++)
                {
                    cout << fields[i].name << "=" << row[i] << endl;
                }
            }
        }
        mysql_free_result(res);

    }
    mysql_close(&mysql);
}


int main () {
    init_db();

    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:8787");

    while (true) {

		string tag_req;
		login_req_pk login_req;
		recv_data(socket, tag_req, login_req);
		cout << tag_req << login_req.user << endl;
		
		const string tag_rsp = "login";
		login_rsp_pk login_rsp;
		login_rsp.err_code = 0;
		send_data(socket, tag_rsp, login_rsp);

    }
    return 0;
}



