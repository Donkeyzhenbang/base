#pragma once
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

const std::string PATH = "/home/jym";
const int PROJ_ID = 999;
const int BUFF_SIZE = 1024;

enum
{
    MSG_CREAT_ERR = 1,
    MSG_GET_ERR,
    MSG_DELETE_ERR
};

int CreateMsg()
{
    key_t key = ftok(PATH.c_str(), PROJ_ID);
    int msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (msgid < 0)
    {
        perror("msg create error");
        exit(MSG_CREAT_ERR);
    }
    return msgid;
}

int GetMsg()
{
    key_t key = ftok(PATH.c_str(), PROJ_ID);
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        perror("msg get error");
        exit(MSG_GET_ERR);
    }
    return msgid;
}
