#ifndef DB_HPP
#define DB_HPP

#include <dat_http/cgi.hpp>
#include <dat_http/session.hpp>
#include <dat_http/params.hpp>
#include <dat_http/status.hpp>
#include <dat_base/except.hpp>
#include <dat_base/maybe.hpp>

#include <dat_db/ddb.hpp>
#include <iostream>     



using namespace std;
using namespace dat_base;
using namespace dat_http;
using namespace dat_db;

struct Message {
    string id;
    string from, to;
    string subject, content;
    long time;
};

Maybe<string> userByName(string _name);
string getSequence();
Maybe<string> userNameById(string _id);

Maybe<string> login(string username, string password);
void deleteMessages (list<string> ids);
bool send(string _from, string _to, string _subject, string _content);
list<Message> messagesByTo(string toId);
Maybe<Message> messageById(string id);

#endif
