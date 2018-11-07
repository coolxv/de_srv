#include <unistd.h>
#include <time.h>
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
static void recv_tag(zmq::socket_t& socket, string& tag)
{
	zmq::message_t tag_msg;
	socket.recv(&tag_msg);
	//tag
	string tag_r(static_cast<const char*>(tag_msg.data()), tag_msg.size());
	tag = tag_r;

}
template <typename T>
static void recv_data(zmq::socket_t& socket, T& data)
{
	zmq::message_t body_msg;
	socket.recv(&body_msg);
	//data
	msgpack::unpacked unpacked_body = msgpack::unpack(static_cast<const char*>(body_msg.data()), body_msg.size());
	msgpack::object deserialized = unpacked_body.get();
	deserialized.convert(data);
}



int  init_db( MYSQL &mysql)
{
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql, "localhost", "root", "Coolxv1818518", NULL, 3306, NULL, 0))
    {
        mysql_close(&mysql);
        cout << mysql_error(&mysql) << endl;
        return 0;
    }

    mysql_set_character_set(&mysql, "utf8");
    char value = 1;
    mysql_options(&mysql, MYSQL_OPT_RECONNECT, &value);

    mysql_autocommit(&mysql, 1);
    
    if(0 != mysql_select_db(&mysql, "gp"))
    {
        cout << mysql_error(&mysql) << endl;
        return 0;
    }
#if 0
    string sql = "select * from user";
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
#endif
    return 1;
}

static int check_pwd_for_login(MYSQL &mysql, const login_req_pk &login_req)
{

    MYSQL_RES *res;
    MYSQL_ROW row;

    my_ulonglong num_rows;
    unsigned int num_fields;

    string sql = "select pwd, status from user where user=" + login_req.user;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if((num_rows == 1) 
                && (0 == strcmp(row[0], login_req.pwd.c_str()))
                && (row[1] == 1))
            {
                return 1;
            }
            mysql_free_result(res);
        }
    }

    return 0;


}

static int check_date_for_login(MYSQL &mysql, const login_req_pk &login_req)
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    my_ulonglong num_rows;
    unsigned int num_fields;

    string db_date;
    string sql = "select expire_date from uuid where user=" + login_req.user + " uuid=" + login_req.uuid;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if(num_rows == 1) 
            {
                db_date = row[0];
            }
            else
            {
                mysql_free_result(res);
                return 0;
            }
            mysql_free_result(res);
        }
    }
    else
    {
        return 0;
    }
    struct tm tm_time;
    strptime(db_date.c_str(), "%Y-%m-%d %H:%M:%S", &tm_time);
    time_t  expire_date =  mktime(&tm_time);
    time_t current_date = time(NULL);
    if(expire_date > current_date)
    {
        return 0
    }
    
    return 1;



    return 1;


}


static int check_count_for_login(MYSQL &mysql, const login_req_pk &login_req)
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    my_ulonglong num_rows;
    unsigned int num_fields;

    int count = 0;
    string sql = "select count from uuid where user=" + login_req.user + " uuid=" + login_req.uuid;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if(num_rows == 1) 
            {
                count = row[0];
            }
            else
            {
                mysql_free_result(res);
                return 0;
            }
            mysql_free_result(res);
        }
    }
    else
    {
        return 0;
    }

    return 0;


}


static void proc_login(MYSQL &mysql, zmq::socket_t& socket, const login_req_pk &login_req)
{
    const string tag_rsp = "login";

    if(0 == check_pwd_for_login(mysql, login_req))
    {
        login_rsp_pk login_rsp;
        login_rsp.err_code = 0;
        login_rsp.err_msg = "pwd error";
        send_data(socket, tag_rsp, login_rsp);
        return;
    }





}
static void proc_logout(MYSQL &mysql, zmq::socket_t& socket, const logout_req_pk &logout)
{
    

}


int main ()
{
    //init db
    MYSQL mysql;
    init_db(mysql);
    //init socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
    socket.bind ("tcp://*:8787");
    //loop process package
    while (true) {

        string tag_req;
        recv_tag(socket, tag_req);
        if(tag_req == "login")
        {
            login_req_pk login_req;
            recv_data(socket, tag_req, login_req);
            cout << tag_req << ":" << login_req.user << "-" << login_req.uuid << endl;
            proc_login(mysql, socket, login_req);
        }
        else if(tag_req == "logout")
        {
            logout_req_pk logout_req;
            recv_data(socket, logout_req);
            cout << tag_req << ":" << logout_req.user << "-" << logout_req.uuid << endl;
            proc_logout(mysql, socket, logout_req);
        }
        else
        {
            cout << tag_req << ":" <<  "not process" << endl;
        }

    }
    return 0;
}



