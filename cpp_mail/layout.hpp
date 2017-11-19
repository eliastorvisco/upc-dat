#ifndef LAYOUT_HPP
#define LAYOUT_HPP

#include <dat_http/cgi.hpp>
#include <dat_http/session.hpp>
#include <dat_http/params.hpp>
#include <dat_http/status.hpp>
#include <dat_base/except.hpp>
#include <dat_base/maybe.hpp>

#include <iostream>     

using namespace std;
using namespace dat_base;
using namespace dat_http;

string escapeHtml(const string& text);
ByteString layout(Maybe<string> mbUserId, string title, ByteString body);
ByteString inboxPage(string userId);
ByteString loginPage();
ByteString sendPage(string userId);
ByteString messagePage(string mid, string userId);

#endif
