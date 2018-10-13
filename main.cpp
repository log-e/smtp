#include "smtp.h"

int main()
{
    MailBox mybox;
    mybox.setServerName("smtp.qq.com");
    mybox.setServerPort(25);
    mybox.setSender_username("xx@xx.com");
    mybox.setSender_addr("xx@xx.com");
    mybox.setSubject("test");
    mybox.addReceiver("xxx@cc.com");
    mybox.setSender_password("12345678");
    mybox.addContent("hello email");
    mybox.addAttachment("我爱你.jpg");
    mybox.sendMail();
    return 0;
}