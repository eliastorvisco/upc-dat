#include "layout.hpp"
#include "db.hpp"

using namespace std;
using namespace dat_base;
using namespace dat_http;

string escapeHtml(const string& text) {
        
        string result;
        for (unsigned i = 0; i < text.size(); i++) {
                char c = text[i];
                switch (c) {
                        case '<': result += "&lt;"; break;
                        case '&': result += "&amp;";break;
                        case '>':result += "&gt;"; break;
                        case '\"':result += "&quot;"; break;
                        case '\'':result += "&apos;"; break;
                        default: result += c; break;
                }
        }
        return result;
}

ByteString layout(Maybe<string> mbUserId, string title, ByteString body) {

	ByteString html;
	html.append("<html><head><meta charset='UTF-8'>\n")
		.append("<title>Pràctica 2: Missatgeria</title>\n")
		.append("<link rel='stylesheet' href='https://cdn.jsdelivr.net/semantic-ui/2.2.9/semantic.min.css'>\n")
		.append("<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta.2/css/bootstrap.min.css' integrity='sha384-PsH8R72JQ3SOdhVi3uxftmaW6Vc51MKb0q5P2rRUpPvrszuE4W1povHYgTpBfshb' crossorigin='anonymous'>\n")
		.append("</head>\n")
                .append("<body role='flatdoc'>\n")
                .append("<div>")
                .append("<div class='ui top fixed menu'><div class='item'><h4>DAT ldatusr12</h4>")
                .append("</div>")
                .append("<a href='/~ldatusr12/practica1' class='item'>Pràctica 1</a>")
                .append("<a href='/~ldatusr12/practica2/mail.cgi/' class='item '>Pràctica 2</a>")
                .append("</div>")
		.append(body)
		.append("</body></html>\n");

	return html;
 }

 ByteString inboxPage(string userId) {

	ByteString html;
	list<Message> messages = messagesByTo(userId);

        html.append("<div class='ui container' style='padding-top: 60px;height:95%'>")
            .append("<div class='row page-header' >")
            .append("<h1>SISTEMA DE MISSATGERIA</h1>")
            .append("</div>")
            .append("<div class='row page-header'  ><form method='post' action='http://soft0.upc.edu/~ldatusr12/practica2/mail.cgi/logout'>")
            .append("<span class='glyphicon glyphicon-user'></span>Usuari: <b>"+escapeHtml(fromJust(userNameById(userId)))+"</b>&nbsp;&nbsp;")
            .append("<span class='glyphicon glyphicon-log-out'></span><button type='submit' class='btn btn-link' name='logout'>Tanca sessió</button>")
            .append("</form></div>")
            .append("<form class='col-xl-12' action='' method='post' id='first-form'>")
            .append("<div class='col-xl-12' style='overflow-y:auto; height: 65%'>")
            .append("<table class='table table-striped table-condensed'>")
            .append("<thead><tr><th>&nbsp;</th> <th>De</th> <th>Data</th> <th>Assumpte</th></tr></thead>")
            .append("<tbody>");
         
	for (auto mit = messages.cbegin(); mit != messages.cend(); ++mit) {
                
                Maybe<string> mbFrom = userNameById(mit->from);
                time_t date = mit->time;
                string sdate = asctime(localtime(&date));

                html.append("<tr><td><input type=checkbox name='deleteId' value='"+escapeHtml(mit->id)+"'></td>")
                    .append("<td><em>"+escapeHtml(fromJust(mbFrom))+"</em></td>")
                    .append("<td>"+escapeHtml(sdate)+"</td>")
                    .append("<td><a href='http://soft0.upc.edu/~ldatusr12/practica2/mail.cgi/inbox/"+escapeHtml(mit->id)+"'>"+escapeHtml(mit->subject)+"</a></td></tr>");
                
	 }
	 
        html.append("</tbody>")
            .append("</table>")
            .append("</div><div style='height:15%'>")
            .append("<a href='send' class='btn btn-primary' style='margin-right: 10px'>Send</a>")
            .append("<button class='btn btn-danger'name='delete' value='delete' type='submit'>Delete</button>")
            .append("</div></form></div>");

        return html;
 }

 ByteString loginPage() {

	ByteString html;
        html.append("<div class='ui container' style='padding-top: 60px'>\n")
            .append("<form class='ui form' method='POST'>\n")
            .append("<div class='field'>\n")
            .append("<label>Usuari</label>\n")
            .append("<input type='text' name='username' placeholder='Usuari'>\n")
            .append("</div>\n")
            .append("<div class='field'>\n")
            .append("<label>Contrasenya</label>\n")
            .append("<input type='password' name='password' placeholder='Contrasenya'>\n")
            .append("</div>\n")
            .append("<button class='ui button' type='submit' name='login'>Submit</button>\n")
            .append("</form></div>\n");

        return html;
 } 

 ByteString sendPage(string userId) {
        ByteString html;
        html.append("<div class='ui container' style='padding-top: 60px'>")
            .append("<form action='' class='ui form'>")
            .append("<input type='hidden' name='from' value='" + escapeHtml(userId) + "'>")
            .append("<div class='field'>")
            .append("<label for='to'>Destinatari</label>")
            .append("<input type='text' name='to' placeholder='e.g. usuari1...'>")
            .append("</div>")
            .append("<div class='field'>")
            .append("<label for='subject'>Assumpte</label>")
            .append("<input type='text' name='subject' placeholder=''>")
            .append("</div>")
            .append("<div class='field'>")
            .append("<label for='body'>Contingut</label>")
            .append("<textarea name='body' id='' cols='30' rows='20'></textarea>")
            .append("</div>")
            .append("<input type='hidden' name='userId' value='1'>")
            .append("<button formmethod='post' type='submit' name='send' value='send' class='ui primary button'>Send</button>")
            .append("</form>")
            .append("</div>");

        return html;
 }
 
 ByteString messagePage(string mid, string userId) {
        
        ByteString html;
        Maybe<Message> mMessage = messageById(mid);
        Message message = fromJust(mMessage);

        html.append("<div style='height:13%'></div>")
            .append("<div class='ui container' style='word-wrap: break-word;'>")
            .append("<div class='jumbotron' style='height:10%; padding:10px; font-size:2em'><b>"+fromJust(userNameById(message.from))+"</b>: " + message.subject+"</div>")
            .append("<div class='jumbotron' style='height:50%;padding:20px; margin-top: 10px; overflow-y:auto'>"+message.content +"</div>")
            .append("<form ><div style='height:15%'>")
            .append("<input type='hidden' name='deleteId' value='"+escapeHtml(mid)+"'>")
            .append("<button formmethod='POST' formaction='/~ldatusr12/practica2/mail.cgi/inbox' class='btn btn-danger'name='delete' value='delete' type='submit'>Delete</button>")
            .append("</div></form></div>");

        return html;
 }
 
 
 

