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
bool send_data(zmq::socket_t& socket, const string& tag, const T& data)
{
	msgpack::sbuffer packed;
	msgpack::pack(&packed, data);
	//tag
	zmq::message_t tag_msg(tag.size());
	std::memcpy(tag_msg.data(), tag.data(), tag.size());
	bool ret1 = socket.send(tag_msg, ZMQ_SNDMORE);
	//data
	zmq::message_t body_msg(packed.size());
	std::memcpy(body_msg.data(), packed.data(), packed.size());
	bool ret2 = socket.send(body_msg);

	return ret1 && ret2;

}

static bool recv_tag(zmq::socket_t& socket, string& tag)
{
	zmq::message_t tag_msg;
	bool ret = socket.recv(&tag_msg);
	//tag
	if (ret)
	{
		tag.assign(static_cast<const char*>(tag_msg.data()), tag_msg.size());
	}
	return ret;

}


template <typename T>
static bool recv_data(zmq::socket_t& socket, T& data)
{
	zmq::message_t body_msg;
	bool ret = socket.recv(&body_msg);
	//data
	if (ret)
	{
		msgpack::unpacked unpacked_body = msgpack::unpack(static_cast<const char*>(body_msg.data()), body_msg.size());
		msgpack::object deserialized = unpacked_body.get();
		deserialized.convert(data);
	}
	return ret;
}



static string  get_code_from_machine(string machine)
{
#define MACHINE_CODE_BUFFER_SIZE 21
#define CODE_BUFFER_SIZE 11

	if (machine.length() != (MACHINE_CODE_BUFFER_SIZE - 1))
	{
		return "";
	}
	char code[CODE_BUFFER_SIZE];
	for (int i = 0, j = 0; i < (MACHINE_CODE_BUFFER_SIZE-1) && i < (int)machine.length() ; i++,j++)
	{
		code[j] = machine[++i];
	}
	code[CODE_BUFFER_SIZE - 1] = 0;
	return code;

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

    return 1;
}

static int check_user_for_login(MYSQL &mysql, const login_req_pk &login_req)
{

    MYSQL_RES *res;
    MYSQL_ROW row;

    my_ulonglong num_rows;
    unsigned int num_fields;

    string sql = "select pwd, status from user where user='" + login_req.user + "'";
	//cout << sql << endl;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if((num_rows == 1) 
                && (0 == strcmp(row[0], login_req.pwd.c_str()))
                && (atoi(row[1]) == 1))
            {
                return 1;
            }
            mysql_free_result(res);
        }
    }
	else
	{
		cout << mysql_error(&mysql) << endl;
	}

    return 0;


}

static int check_uuid_for_login(MYSQL &mysql, const login_req_pk &login_req,string &expire_date)
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    my_ulonglong num_rows;
    unsigned int num_fields;

    string db_expire_date;
    string db_version;
	int db_status;
	string db_price;
    string sql = "select status,version,expire_date,price from uuid where user='" + login_req.user + "' and uuid='" + login_req.uuid + "'";
	//cout << sql << endl;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if(num_rows == 1) 
            {
            	db_status = atoi(row[0]);
	            db_version = row[1];
                db_expire_date = row[2];
				db_price = row[3];
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
		cout << mysql_error(&mysql) << endl;
        return 0;
    }

	//check status
    if(db_status != 1 )
    {
        return 0;
    }
	//check version
	int db_ver_i = atoi(db_version.c_str());
	int pk_ver_i = atoi(login_req.ver.c_str());
    if(pk_ver_i < db_ver_i)
    {
        return 0;
    }
	//check price
	int db_price_i = atoi(db_price.c_str());
	int pk_price_i = atoi(login_req.price.c_str());
    if(db_price_i > 0 && (pk_price_i < db_price_i || pk_price_i > 200))
    {
        return 0;
    }

	//check date
    struct tm tm_time;
    strptime(db_expire_date.c_str(), "%Y-%m-%d %H:%M:%S", &tm_time);
    time_t  db_expire_date_t =  mktime(&tm_time);
    time_t current_date = time(NULL);
    if(current_date > db_expire_date_t)
    {
        return 0;
    }
    expire_date = db_expire_date;
    return 1;


}


static int check_count_for_login(MYSQL &mysql, const login_req_pk &login_req)
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    my_ulonglong num_rows;
    unsigned int num_fields;

    int count = 0;
	//get count
    string sql = "select count from uuid where user='" + login_req.user + "' and uuid='" + login_req.uuid + "'";
	//cout << sql << endl;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if(num_rows == 1) 
            {
                count = atoi(row[0]);
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
		cout << mysql_error(&mysql) << endl;
        return 0;
    }
	//compute count
    int ccount = 0;
    sql = "select count(*) from mc where user='" + login_req.user + "' and uuid='" + login_req.uuid + "'";
	//cout << sql << endl;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if(num_rows == 1) 
            {
                ccount = atoi(row[0]);
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
		cout << mysql_error(&mysql) << endl;
        return 0;
    }

	//if exist
    int cccount = 0;
    sql = "select count(*) from mc where user='" + login_req.user + "' and uuid='" + login_req.uuid + "' and mc='" + get_code_from_machine(login_req.mc) + "'";
	//cout << sql << endl;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if(num_rows == 1) 
            {
                cccount = atoi(row[0]);
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
		cout << mysql_error(&mysql) << endl;
        return 0;
    }

	if(ccount < count)
	{
		return 1;
	}
	else if(count == ccount && 1 == cccount)
	{
		return 1;
	}

    return 0;


}

static int add_or_update_mc_for_login(MYSQL &mysql, const login_req_pk &login_req)
{

    MYSQL_RES *res;
    MYSQL_ROW row;

    my_ulonglong num_rows;
    unsigned int num_fields;

	//if exist
    int count = 0;
    string sql = "select count(*) from mc where user='" + login_req.user + "' and uuid='" + login_req.uuid + "' and mc='" + get_code_from_machine(login_req.mc) + "'";
	//cout << sql << endl;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if(num_rows == 1) 
            {
                count = atoi(row[0]);
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
		cout << mysql_error(&mysql) << endl;
        return 0;
    }
	//insert into mc (user,uuid,mc,status,pub_ip,pri_ip,ver,login_date) values ('15011457740','db1ac97cf2bb5bab8481b0614346852f','zdfwdertaf',1,'2.2.2.2','192.168.1.1','1.0.0',now())
	if(0 == count)
	{
		sql = "insert into mc (user,uuid,mc,status,mn,pub_ip,pri_ip,mac,price,ver,login_date,create_date) values('" 
			+ login_req.user
			+ "','" + login_req.uuid
			+ "','" + get_code_from_machine(login_req.mc)
			+ "'," + "1"
			+ ",'" + login_req.mn
			+ "'," + "''"
			+ ",'" + login_req.ip
			+ "','" + login_req.mac
			+ "','" + login_req.price
			+ "','" + login_req.ver
			+ "'," + "now()"
			+ "," + "now())";
		cout << sql << endl;
		if(0 != mysql_real_query(&mysql, sql.c_str(), sql.size()))
		{
			cout << mysql_error(&mysql) << endl;
			return 0;
		}

	}
	//update mc set login_date=now() where user=15011457740 and uuid='db1ac97cf2bb5bab8481b0614346852f' and mc='zdfwdertaf'
	else if(1 == count)
	{
		sql = "update mc set status=1, action=2, pri_ip='" + login_req.ip + "', mac='"  + login_req.mac  + "', ver='"  + login_req.ver  + "', price='"  + login_req.price + "', login_date=now() where user='" + login_req.user + "' and uuid='" + login_req.uuid + "' and mc='" + get_code_from_machine(login_req.mc) + "'";
		cout << sql << endl;
		if(0 != mysql_real_query(&mysql, sql.c_str(), sql.size()))
		{
			cout << mysql_error(&mysql) << endl;
			return 0;
		}

	}
    return 1;


}

static int update_mc_for_logout(MYSQL &mysql, const logout_req_pk &logout_req)
{

    MYSQL_RES *res;
    MYSQL_ROW row;

    my_ulonglong num_rows;
    unsigned int num_fields;

	//if exist
    int count = 0;
    string sql = "select count(*) from mc where user='" + logout_req.user + "' and uuid='" + logout_req.uuid + "' and mc='" + get_code_from_machine(logout_req.mc) + "'";
	//cout << sql << endl;
    if(0 == mysql_real_query(&mysql, sql.c_str(), sql.size()))
    {
        res = mysql_store_result(&mysql);
        if(nullptr != res)
        {
            num_rows = mysql_num_rows(res);
            row = mysql_fetch_row(res);
            if(num_rows == 1) 
            {
                count = atoi(row[0]);
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
		cout << mysql_error(&mysql) << endl;
        return 0;
    }
	if(1 == count)

	{
		sql = "update mc set status=0, action=3, logout_date=now() where user='" + logout_req.user + "' and uuid='" + logout_req.uuid + "' and mc='" + get_code_from_machine(logout_req.mc) + "'";
		//cout << sql << endl;
		if(0 != mysql_real_query(&mysql, sql.c_str(), sql.size()))
		{
			cout << mysql_error(&mysql) << endl;
			return 0;
		}

	}
    return 1;


}

static int proc_login(MYSQL &mysql, zmq::socket_t& socket, const login_req_pk &login_req)
{
    const string tag_rsp = "login";
	string expire_date;
	
    if(0 == check_user_for_login(mysql, login_req))
    {
        login_rsp_pk login_rsp;
        login_rsp.err_code = 0;
        login_rsp.err_msg = "user error";
        send_data(socket, tag_rsp, login_rsp);
		cout << "login user error" << ":" << login_req.user << "-" << login_req.uuid << "-" << login_req.mc << "-" << login_req.mn << "-" << login_req.ip << "-" << login_req.ver << endl;
        return 0;
    }
	
    if(0 == check_uuid_for_login(mysql, login_req, expire_date))
    {
        login_rsp_pk login_rsp;
        login_rsp.err_code = 0;
        login_rsp.err_msg = "uuid error";
        send_data(socket, tag_rsp, login_rsp);
		cout << "login uuid error" << ":" << login_req.user << "-" << login_req.uuid << "-" << login_req.mc << "-" << login_req.mn << "-" << login_req.ip << "-" << login_req.ver <<  endl;
        return 0;
    }
    if(0 == check_count_for_login(mysql, login_req))
    {
        login_rsp_pk login_rsp;
        login_rsp.err_code = 0;
        login_rsp.err_msg = "count limit";
        send_data(socket, tag_rsp, login_rsp);
		cout << "login count limit" << ":" << login_req.user << "-" << login_req.uuid << "-" << login_req.mc << "-" << login_req.mn << "-" << login_req.ip << "-" << login_req.ver <<  endl;
        return 0;
    }

	add_or_update_mc_for_login(mysql, login_req);
	//
	login_rsp_pk login_rsp;
	login_rsp.err_code = 1;
	login_rsp.err_msg = "login sucess";
	login_rsp.date = expire_date;
	send_data(socket, tag_rsp, login_rsp);

	cout << "login sucess" << ":" << login_req.user << "-" << login_req.uuid << "-" << login_req.mc << "-" << login_req.mn << "-" << login_req.ip << "-" << login_req.ver <<  endl;
	return 1;
}
static int proc_logout(MYSQL &mysql, zmq::socket_t& socket, const logout_req_pk &logout_req)
{
    const string tag_rsp = "logout";
    
	update_mc_for_logout(mysql, logout_req);
	//
	logout_rsp_pk logout_rsp;
	logout_rsp.err_code = 1;
	logout_rsp.err_msg = "logout sucess";
	send_data(socket, tag_rsp, logout_rsp);
	cout << "logout sucess" << ":" << logout_req.user << "-" << logout_req.uuid << "-" << logout_req.mc << "-" << logout_req.mn << "-" << logout_req.ip << "-" << logout_req.ver <<  endl;
	return 1;
}


void main_loop (MYSQL &mysql)
{

    //init socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, ZMQ_REP);
	int sendtimeout = 3000;
	socket.setsockopt(ZMQ_SNDTIMEO, &sendtimeout, sizeof(sendtimeout));
	int lingertime = 0;
	socket.setsockopt(ZMQ_LINGER, &lingertime, sizeof(lingertime));
    socket.bind ("tcp://*:8888");
    //loop process package
    while (true) {

        string tag_req;
        recv_tag(socket, tag_req);
        if(tag_req == "login")
        {
            login_req_pk login_req;
            recv_data(socket, login_req);
            int ret = proc_login(mysql, socket, login_req);
        }
        else if(tag_req == "logout")
        {
            logout_req_pk logout_req;
            recv_data(socket, logout_req);
            int ret = proc_logout(mysql, socket, logout_req);
        }
        else
        {
            cout << tag_req << ":" <<  "not process" << endl;
        }

    }
}

int main ()
{

    //init db
    MYSQL mysql;
    int ret = init_db(mysql);
	if(ret > 0)
	{
		while(true)
		{
			try
			{
				main_loop(mysql);
			}
			catch (exception& e)
			{
				cout << "exception:" << e.what() <<endl;
			}
		}
	}
	return 0;
}


