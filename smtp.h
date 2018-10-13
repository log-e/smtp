#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

using namespace std;


class MailBox
{

public:
    MailBox();
    ~MailBox();
    MailBox(const char* sender_addr, const char* sender_username,
            const char* sender_password, const char* subject,
            const char* server, int port);

public:
	int setSender_addr(const char* addr_in);
	int setSender_username(const char* username_in);
	int setSender_password(const char* password_in);
	int addReceiver(const char* receiver);
	int setSubject(const char* subject);
	int setServerName(const char* server);
	int setServerPort(short port = 25);
	int addContent(const char* content);
	int addAttachment(const char* attachContent);
	int sendMail();


private:
	string m_addr;
	string m_username;
	string m_password;
	vector<string> m_recevier;
	string m_server;
	short m_port;
	vector<string> m_content;
	string m_subject;
    int m_sock;

private:
    int sendRequest(string msg);
    int recvResponse(string response);

private:
    int helo();
    int login();
    int sendHeader();
    int sendContent();
    int sendEnd();
	int conn();
};