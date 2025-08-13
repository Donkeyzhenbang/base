#include "com.hpp"

int main()
{
    int msgid = GetMsg();
    struct msgbuf buffer;
    buffer.mtype = 1;
    while (true)
    {
        std::cout << "Says # ";
        std::string s;
        std::getline(std::cin, s);
        strcpy(buffer.mtext, s.c_str());
        msgsnd(msgid, &buffer, BUFF_SIZE, 0);
    }
    return 0;
}
