#include "smtp.h"


const std::string _base64_encode_chars ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
string encode(const string in_str)
{
    string out_str;
    unsigned char c1, c2, c3;
    int i = 0;
    int len = (int)in_str.length();

    while ( i<len )
    {
        // read the first byte
        c1 = in_str[i++];
        if ( i==len )       // pad with "="
        {
            out_str += _base64_encode_chars[ c1>>2 ];
            out_str += _base64_encode_chars[ (c1&0x3)<<4 ];
            out_str += "==";
            break;
        }

        // read the second byte
        c2 = in_str[i++];
        if ( i==len )       // pad with "="
        {
            out_str += _base64_encode_chars[ c1>>2 ];
            out_str += _base64_encode_chars[ ((c1&0x3)<<4) | ((c2&0xF0)>>4) ];
            out_str += _base64_encode_chars[ (c2&0xF)<<2 ];
            out_str += "=";
            break;
        }

        // read the third byte
        c3 = in_str[i++];
        // convert into four bytes string
        out_str += _base64_encode_chars[ c1>>2 ];
        out_str += _base64_encode_chars[ ((c1&0x3)<<4) | ((c2&0xF0)>>4) ];
        out_str += _base64_encode_chars[ ((c2&0xF)<<2) | ((c3&0xC0)>>6) ];
        out_str += _base64_encode_chars[ c3&0x3F ];
    }

    return out_str;
}


MailBox::MailBox() {
    m_sock = socket(AF_INET, SOCK_STREAM, 0);
}

MailBox::~MailBox() {
}

int MailBox::setSender_addr(const char *addr_in) {
    m_addr = addr_in;
    return 0;
}

int MailBox::setSender_username(const char *username_in) {
    m_username = username_in;
    return 0;
}

int MailBox::setSender_password(const char *password_in) {
    m_password = password_in;
    return 0;
}

int MailBox::addReceiver(const char *receiver) {
    m_recevier.push_back(receiver);
    return 0;
}

int MailBox::setSubject(const char *subject) {
    m_subject = subject;
    return 0;
}

int MailBox::setServerName(const char *server) {
    m_server = server;
    return 0;
}

int MailBox::setServerPort(short port) {
    m_port = port;
    return 0;
}

int MailBox::addContent(const char *content) {
    //todo
    string msg;
    msg = "--";
    msg += "@@";
    msg += "\r\n";
    msg += "Content-Type: text/plain;";
    msg += "charset=\"utf-8\"";
    msg += "\r\n\r\n";
    msg += content;
    msg += "\r\n\r\n";
    m_content.push_back(msg);
    return 0;
}

int MailBox::addAttachment(const char *attachContent) {

    string msg;
    msg = "--@@\r\n";
    msg += "Content-Type: application/octet-stream; name=";
    msg += attachContent;
    msg += "\r\n";

    msg += "Content-Transfer-Encoding: base64\r\n";
    msg += "Content-Dispotion: attachment; filename=";
    msg += attachContent;
    msg += "\r\n\r\n";

    int fd = -1;
    const int size = 4096;
    char buffer[size] = {0};
    string tmp;
    string tmpC = "";
    int len = 0;
    if((fd = open(attachContent, O_RDONLY)) == -1)
    {
        perror("cannot open the file\n");
    }

    while((len = read(fd, buffer, size)) > 0)
    {
        tmp.assign(buffer, len);
        tmpC += tmp;
    }
    tmpC = encode(tmpC);
    msg += tmpC;
    msg += "\r\n\r\n";
    m_content.push_back(msg);
    return 0;
}


int MailBox::sendRequest(string msg) {

    ssize_t ret = -1;
    ret = write(m_sock, msg.c_str(), msg.length());
    if(ret < 0)
    {
        printf("send error\n");
        printf("send buf: %s\n", msg.c_str());
        return -1;
    }
    printf("> \n%s", msg.c_str());
    return 0;
}

int MailBox::recvResponse(string response) {
    ssize_t ret = -1;
    char buffer[256] = {0};
    ret = read(m_sock, buffer, 256);
    if(strncmp(response.c_str(), buffer, response.length()) != 0)
    {
        printf("recv error\n");
        return -1;
    }
    printf("<\n%s", buffer);
    return 0;
}

int MailBox::conn() {

    int ret = -1;
    //获取ip
    struct hostent* hptr;
    hptr = gethostbyname(m_server.c_str());
    char ip[32];
    inet_ntop(hptr->h_addrtype, (hptr->h_addr_list)[0], ip, sizeof(ip));
    //准备连接
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(m_port);
    serveraddr.sin_addr.s_addr = inet_addr(ip);
    //开始连接
    if(connect(m_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
    {
        printf("connecting error\n");
        return -1;
    }

    ret = recvResponse("220");
    if (ret < 0)
    {
        printf("220 connecting error\n");
    }
    return 0;
}

int MailBox::helo() {
    ssize_t ret = -1;
    string msg;
    msg = "HELO ";
    msg += m_server;
    msg += "\r\n";

    ret = sendRequest(msg);
    if(ret < 0)
    {

    }

    ret = recvResponse("250");
    if(ret < 0)
    {

    }
    return 0;
}

int MailBox::login() {
    string msg;
    int ret = -1;
    msg = "AUTH LOGIN\r\n";
    ret = sendRequest(msg);
    ret = recvResponse("334");


    //msg = b64encode(m_username);
    msg = encode(m_username);
    msg += "\r\n";
    ret = sendRequest(msg);
    ret = recvResponse("334");

    //msg = b64encode(m_password);
    msg = encode(m_password);
    msg += "\r\n";
    ret = sendRequest(msg);
    ret = recvResponse("235");
    return 0;
}

int MailBox::sendHeader() {
    string msg;
    int ret = -1;
    msg = "MAIL FROM: ";
    msg += m_addr;
    msg += "\r\n";
    ret = sendRequest(msg);
    ret = recvResponse("250");

    for(int i=0; i < m_recevier.size(); i++) {
        msg = "RCPT TO: ";
        msg += m_recevier[i];
        msg += "\r\n";
        ret = sendRequest(msg);
        ret = recvResponse("250");
    }

    msg = "DATA\r\n";
    ret = sendRequest(msg);
    ret = recvResponse("354");

    msg = "From: ";
    msg += m_addr;
    msg += "\r\n";

    for(int i=0; i < m_recevier.size(); i++) {
        msg += "TO: ";
        msg += m_recevier[i];
        if(i == m_recevier.size() - 1)
        {
            msg += "\r\n";
        }
        else
        {
            msg += ",";
        }
    }

    msg += "Subject: ";
    msg += m_subject;
    msg += "\r\n";

    msg += "MIME_Version: 1.0";
    msg += "\r\n";

    msg += "Content-type: multipart/mixed;boundary=";
    msg += "\"";
    msg += "@@";
    msg += "\"";
    msg += "\r\n\r\n";
    ret = sendRequest(msg);
    return 0;
}

int MailBox::sendContent() {
    int ret;
    for(int i=0; i < m_content.size(); i++)
    {
        sendRequest(m_content[i]);
    }
    return 0;
}

int MailBox::sendEnd() {
    string msg;
    msg = "--@@--";
    msg += "\r\n.\r\n";

    sendRequest(msg);
    recvResponse("250");

    msg = "QUIT\r\n";
    sendRequest(msg);
    recvResponse("221");
    return 0;
}

int MailBox::sendMail() {
    conn();
    helo();
    login();
    sendHeader();
    sendContent();
    sendEnd();
    return 0;
}

MailBox::MailBox(const char *sender_addr, const char *sender_username,
                 const char *sender_password, const char *subject,
                 const char *server, int port)
{
    m_addr = sender_addr;
    m_username = sender_username;
    m_password = sender_password;
    m_subject = subject;
    m_server = server;
    m_port = port;
}