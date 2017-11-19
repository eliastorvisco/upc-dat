
#include "db.hpp"


using namespace std;
using namespace dat_base;
using namespace dat_http;

string db_dir("mail-db");

Maybe<string> userByName(string _name);
string getSequence();


Maybe<string> login (string username, string password) {

	DBConnection dbc(db_dir);
	dbc.select("users");

	while(dbc.next())
		if(username == dbc.get("name") && password == dbc.get("password")) 
			return Just(dbc.get("id"));

	return Nothing<string>();
}

void deleteMessages (list<string> ids) {

	DBConnection dbc(db_dir);
	for (auto it = ids.cbegin(); it != ids.cend(); it++) 
		dbc.delete_rows("messages", "id", *it);
		
}

list<Message> messagesByTo(string toId) {
	
	DBConnection dbc(db_dir);
	dbc.select("messages", "to", toId); //Generates temp table with every message with to = toId

	list<Message> messages;

	while(dbc.next()) {

		Message message;
		message.id = dbc.get("id");
		message.from = dbc.get("from");
		message.to = dbc.get("to");
		message.subject = dbc.get("subject");
		message.content = dbc.get("content");
		message.time = stol(dbc.get("time")); //Get it as long

		messages.push_back(message);
	}
	return messages;
	
}

Maybe<Message> messageById(string id) {

	//TODO: Hacer la comprobacion de que el mensaje pertenece a quien lo pide

	DBConnection dbc(db_dir);
	dbc.select("messages", "id", id);
	if (dbc.next()) {

		Message message;
		message.id = dbc.get("id");
		message.from = dbc.get("from");
		message.to = dbc.get("to");
		message.subject = dbc.get("subject");
		message.content = dbc.get("content");
		message.time = stol(dbc.get("time")); //Get it as long

		return Just(message);
	}
	
	return Nothing<Message>();

}

bool send(string _from, string _to, string _subject, string _content) {
	
	Maybe<string> mto = userByName(_to);
	if(isNothing(mto)) 
		return false;
	
	list<string> message;
	long _time = time(nullptr);

	message.push_back(getSequence());
	message.push_back(_from);
	message.push_back(fromJust(mto));
	message.push_back(to_string(_time));
	message.push_back(_subject);
	message.push_back(_content);
	
	DBConnection dbc(db_dir);
	dbc.insert("messages", message);

	
	return true;

		
}

/***************************************************************
 * 
 * Funcions Auxiliars
 * 
 */

 Maybe<string> userByName(string _name) {

	 DBConnection dbc(db_dir);
	 dbc.select("users", "name", _name);

	 if (dbc.next()) 
		 return Just(dbc.get("id"));
	 
	 return Nothing<string>();
 }

 Maybe<string> userNameById(string _id) {

	 DBConnection dbc(db_dir);
	 dbc.select("users", "id", _id);

	 if (dbc.next()) 
		 return Just(dbc.get("name"));
	 
	 return Nothing<string>();
 }
 

 string getSequence() {

	DBConnection dbc(db_dir);
	dbc.select("msg-seq");
	dbc.next();

	string messageId = dbc.get("value");
	messageId = to_string(stoi(messageId) + 1);
	//DBConnecton dbc(db_dir);
	dbc.update("msg-seq", "value", messageId);

	return messageId;

 }
 




