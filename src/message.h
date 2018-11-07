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
  std::string ver;
  std::time_t time;
  MSGPACK_DEFINE(user, pwd, uuid, mn, mc, ip, ver, time);
};
struct login_rsp_pk
{
  int err_code;
  std::string err_msg;
  std::string date;
  MSGPACK_DEFINE(err_code, err_msg, date);
};


struct logout_req_pk
{
  std::string user;
  std::string pwd;
  std::string uuid; //uuid
  std::string mn; //machine name
  std::string mc; //machine code
  std::string ip;
  std::string ver;
  std::time_t time;
  MSGPACK_DEFINE(user, pwd, uuid, mn, mc, ip, ver, time);
};
struct logout_rsp_pk
{
  int err_code;
  std::string err_msg;
  MSGPACK_DEFINE(err_code, err_msg);
};

