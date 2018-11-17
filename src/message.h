#include <string>
#include <ctime>
#include <msgpack.hpp>

struct login_req_pk
{
  std::string user;
  std::string pwd;
  std::string uuid; //uuid
  std::string mn; //machine name
  std::string mc; //machine code
  std::string ip;
  std::string mac;
  std::string ver;
  std::time_t time;
  std::string price;
  MSGPACK_DEFINE(user, pwd, uuid, mn, mc, ip, mac, ver, time, price);
};
struct login_rsp_pk
{
  int err_code;
  std::string err_msg;
  std::string date;
  short int cmd;
  MSGPACK_DEFINE(err_code, err_msg, date, cmd);
};


struct logout_req_pk
{
  std::string user;
  std::string pwd;
  std::string uuid; //uuid
  std::string mn; //machine name
  std::string mc; //machine code
  std::string ip;
  std::string mac;
  std::string ver;
  std::time_t time;
  MSGPACK_DEFINE(user, pwd, uuid, mn, mc, ip, mac, ver, time);
};
struct logout_rsp_pk
{
  int err_code;
  std::string err_msg;
  short int cmd;
  MSGPACK_DEFINE(err_code, err_msg, cmd);
};

