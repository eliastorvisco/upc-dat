#include <dat_http/cgi.hpp>
#include <dat_http/session.hpp>
#include <dat_http/params.hpp>
#include <dat_http/status.hpp>
#include <dat_base/except.hpp>
#include <dat_base/maybe.hpp>

#include <iostream>     

#include "db.hpp"		// Model
#include "layout.hpp"	// View
#include "urls.hpp"

using namespace std;
using namespace dat_base;
using namespace dat_http;

/********************************************************************
 *  CGI Process
 */

Response mailApp(Request request);

int main(int argc, char **argv, char **env) {

	if(getenv("SERVER_NAME") != nullptr && getenv("SCRIPT_NAME") != nullptr) 
		APPROOT = string("http://") + getenv("SERVER_NAME") + getenv("SCRIPT_NAME");

	cgiApp(env, mailApp);
	return 0;
	
}

/********************************************************************
 *  GET/POST processing (CONTROLLER)
 */

Response getLogin(const Request &req);
Response postLogin(const Request &req);
Response getInbox(const Request &req);
Response postInbox(const Request &req);
Response getSend(const Request &req);
Response postSend(const Request &req);
Response getMessage(string mid, const Request &req);

Response logout(const Request &req);
Maybe<string> getCookieUserId(const Request &req);

Response errorResponse(const Status & status, const string& msg);


string APPROOT;

Response mailApp(Request request) {
  	try {
	  
		string pathInfo = request.rawPathInfo();

		if (pathInfo == LOGIN_URL || pathInfo == "/") {
			
			if (request.method() == "GET") 	
				return getLogin(request); 

			if (request.method() == "POST")	
				return postLogin(request); 
			
			errorResponse(methodNotAllowed405, "Invalid request method");

		} else if (pathInfo == LOGOUT_URL) {
			
			return logout(request);

		} else if (pathInfo == INBOX_URL) {
			
			if (request.method() == "GET") 	
				return getInbox(request); 

			if (request.method() == "POST")	
				return postInbox(request); 
				
			errorResponse(methodNotAllowed405, "Invalid request method");

		} else if (pathInfo == SEND_URL) {
			
			if (request.method() == "GET") 	
				return getSend(request); 

			if (request.method() == "POST")	
				return postSend(request); 
			
		errorResponse(methodNotAllowed405, "Invalid request method");

		} else if (pathInfo.compare(0,7,"/inbox/") == 0) {
            string mid = pathInfo.substr(7);
			if (request.method() == "GET")			
				return getMessage(mid, request);	
				
		} else 
			return errorResponse(notFound404, "Page not found: ");
		
	} catch (const LibException& e) {
		return errorResponse(internalServerError500, "LibException: " + e.str());
	} catch (const std::exception& e) {
		return errorResponse(internalServerError500, string("Standard exception: ") + e.what());
	} catch (...) {
		return errorResponse(internalServerError500, "Unknown exception");
	}
        return errorResponse(internalServerError500, "Unknown exception");
}

/****************************************************************/

Response getLogin(const Request &req) {

	pair<SessionMap, SaveSession> s = clientLoadSession(req);
	Maybe<string> mbUserId = getCookieUserId(req);

	if (!isNothing(mbUserId)) { 

		s.first.insert(pair<string, string>("userId", fromJust(mbUserId))); 
		ResponseHeaders hs = s.second(s.first);
		hs.push_front(Header("Location", APPROOT + INBOX_URL));
		return Response(seeOther303, hs, "");

	}

	ResponseHeaders hs;
	hs.push_back(Header("Content-Type", MIME_HTML));
	ByteString html = layout(Nothing<string>(), "Login", loginPage());

	return Response(ok200, hs, html);

} 

Response postLogin(const Request &req) {

	pair<SessionMap, SaveSession> s = clientLoadSession(req);
	Query params = postParams(req);

	if (!hasParam("login", params))  
		return errorResponse(badRequest400, "Unexpected action.");

	Maybe<string> usernameParam = lookupParam("username", params);
	Maybe<string> passwordParam = lookupParam("password", params);

	if (isNothing(usernameParam) || isNothing(passwordParam)) 
		return errorResponse(badRequest400, "Wrong username or password.");
	
	string username = fromJust(usernameParam);
	string password = fromJust(passwordParam);

	Maybe<string> userId = login(username, password); 
	
	if (isNothing(userId))
		return errorResponse(badRequest400, "Could not login.");

	s.first.insert(pair<string, string>("userId", fromJust(userId))); 

	ResponseHeaders hs = s.second(s.first);
	hs.push_front(Header("Location", APPROOT + INBOX_URL));

	return Response(seeOther303, hs, "");

} 

Response logout(const Request& req) {

	pair<SessionMap, SaveSession> session = clientLoadSession(req);
	SessionMap &sessionMap = session.first;
	Query params = postParams(req);
	sessionMap.erase("userId");
	ResponseHeaders hs = session.second(sessionMap);
	hs.push_front(Header("Location", APPROOT + LOGIN_URL));
	return Response(seeOther303, hs, "");

} 

Response getInbox(const Request &req) {

	pair<SessionMap, SaveSession> s = clientLoadSession(req);
	Maybe<string> mbUserId = getCookieUserId(req);

	if (isNothing(mbUserId))
		return errorResponse(badRequest400, "Expired session.");
	
	ResponseHeaders hs = s.second(s.first);
	hs.push_back(Header("Content-Type", MIME_HTML));
	ByteString html = layout(mbUserId, "Inbox", inboxPage(fromJust(mbUserId)));

	return Response(ok200, hs, html);

} 

Response postInbox(const Request &req) {

	Query params = postParams(req);

	pair<SessionMap, SaveSession> s = clientLoadSession(req);
	Maybe<string> mbUserId = getCookieUserId(req);
	if (isNothing(mbUserId))
		return errorResponse(badRequest400, "Invalid session");
	
	if (hasParam("delete", params)) {

		list<string> ids = lookupParams("deleteId", params);

		for (auto it = ids.cbegin(); it != ids.cend(); it++) {
			Maybe<Message> mbMessage = messageById(*it);
			if(!isNothing(mbMessage) && fromJust(mbMessage).to != fromJust(mbUserId))
				return errorResponse(methodNotAllowed405, 
									 "You are trying to delete a message that does not belong to you");
		}
	
		deleteMessages(ids);

	} 

	ResponseHeaders hs = s.second(s.first);
	hs.push_front(Header("Location", APPROOT + INBOX_URL));

	return Response(ok200, hs, "");
	
}

Response getSend(const Request &req) {
	
	pair<SessionMap, SaveSession> s = clientLoadSession(req);
	Maybe<string> mbUserId = getCookieUserId(req);

	if (isNothing(mbUserId))
		return errorResponse(badRequest400, "Expired session.");
	
	ResponseHeaders hs;
	hs.push_back(Header("Content-Type", MIME_HTML));
	ByteString html = layout(mbUserId, "Send", sendPage(fromJust(mbUserId)));

	return Response(temporaryRedirect307, hs, html);

}

Response postSend(const Request &req) {

	pair<SessionMap, SaveSession> s = clientLoadSession(req);
	Maybe<string> mbUserId = getCookieUserId(req);

	if (isNothing(mbUserId))
		return errorResponse(badRequest400, "Session Expired.");

	Query params = postParams(req);

	if(hasParam("send", params)) {

		Maybe<string> mfrom = lookupParam("from", params);
		Maybe<string> mto = lookupParam("to", params);
		Maybe<string> msubject = lookupParam("subject", params);
		Maybe<string> mcontent = lookupParam("body", params);

		if(isNothing(mfrom) || isNothing(mto) || isNothing(msubject) || isNothing(mcontent))
			return errorResponse(internalServerError500, "There was a problem retrieving the message.");
		
		string from = fromJust(mfrom);
		string to = fromJust(mto);
		string subject = fromJust(msubject);
		string content = fromJust(mcontent);

		if(from.empty() || to.empty() || subject.empty() || content.empty())
			return errorResponse(badRequest400, "Fill every form field to send the message.");

		if(send(from, to, subject, content)) {

			ResponseHeaders hs = s.second(s.first);
			hs.push_front(Header("Location", APPROOT + INBOX_URL));

			return Response(seeOther303, hs, "");

		} else {

			return errorResponse(internalServerError500, "Could not send the message.");

		}
	}

	return errorResponse(badRequest400, "Unexpected Action.");
}

Response getMessage(string mid, const Request &req){

	pair<SessionMap, SaveSession> s = clientLoadSession(req);

	Maybe<string> mbUserId = getCookieUserId(req);
	if (isNothing(mbUserId))
		return errorResponse(badRequest400, "Expired session.");

    Maybe<Message> mMessage = messageById(mid);
    if(isNothing(mMessage)) 
		return errorResponse(internalServerError500, "Could not find the message with id: " + mid);
        
    if(fromJust(mMessage).to != fromJust(mbUserId)) 
		return errorResponse(methodNotAllowed405, "You are trying to access a message that does not belong to you");
	
	ResponseHeaders hs;
	hs.push_back(Header("Content-Type", MIME_HTML));
	ByteString html = layout(mbUserId, "Message", messagePage(mid, fromJust(mbUserId)));
	return Response(ok200, hs, html);

}


/****************************************************************
 * Metodes auxiliars
 */

Maybe<string> getCookieUserId(const Request &req) {
	pair<SessionMap, SaveSession> s = clientLoadSession(req);
	return (s.first.count("userId") == 0)? Nothing<string>() : Just(s.first.at("userId"));
}


/****************************************************************
 * Handlers d'errors
 */

Response errorResponse(const Status & status, const string& msg)
{
	ResponseHeaders hs = list<Header>(1, Header("Content-Type", MIME_HTML));
	ByteString html;
	html.append("<html><head><meta charset=\"UTF-8\">\n").
		append("<title>Pràctica 2: Mail</title>\n").
		append("</head><body>\n").
		append("<h2>ERROR</h2>").
		append("<h3><font color=\"red\">").append(msg).append("</font></h3>\n").
		append("<p><a href=\"/~ldatusr12/practica2/mail.cgi/login\">Anar a la pàgina de inbox.</a></p>\n").
		append("</body></html>\n");
	return Response(status, hs, html);
}
