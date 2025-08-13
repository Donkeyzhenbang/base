#include "com.hpp"

int main()
{
    struct msgbuf buffer;
    int msgid = CreateMsg();
    while (true)
    {
        msgrcv(msgid, &buffer, BUFF_SIZE, 1, 0);
        std::cout << "Client say@ " << buffer.mtext << std::endl;
    }
    return 0;
}
