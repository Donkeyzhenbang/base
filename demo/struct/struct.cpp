#include <iostream>
#include <stddef.h> // 或者 <cstddef>，包含offsetof宏
typedef unsigned char u_int8;
typedef unsigned short u_int16;
typedef unsigned int u_int32;
struct ProtocolHeader_heartbeat {
    uint16_t sync; // 报文头：5AA5H
    uint16_t packetLength; // 报文长度
    char cmdId[17]; // CMD_ID，17位编码
    uint8_t frameType; // 帧类型，09H
    uint8_t packetType; // 报文类型，E6H
    uint8_t frameNo; // 帧序列号
    uint32_t clocktimeStamp; // 当前时间
    uint16_t CRC16;
};

//手动请求拍摄照片报文的响应报文格式
struct __attribute__((packed)) ProtocolB342
{
    u_int16 sync;
    u_int16 packetLength;
    char cmdId[17];
    u_int8 frameType;
    u_int8 packetType;
    u_int8 frameNo;
    u_int8 commandStatus;
    u_int16 CRC16;
    u_int8 End;
};

//监拍装置请求上送照片报文格式
struct __attribute__((packed)) ProtocolB351
{
    u_int16 sync;
    u_int16 packetLength;
    char cmdId[17];
    u_int8 frameType;
    u_int8 packetType;
    u_int8 frameNo;
    u_int8 channelNo;
    u_int8 packetHigh;
    u_int8 packetLow;
    char reverse[8] = {0};
    u_int16 CRC16;
    u_int8 End;
};

//远程图像数据上送结束标记数据报文B37
struct __attribute__((packed)) ProtocolB37
{
    u_int16 sync;        //报文头：5AA5H
    u_int16 packetLength;//报文长度：72字节
    char cmdId[17];  //CMD_ID,17位编码
    u_int8 frameType;    //帧类型，05H（远程图像数据报）
    u_int8 packetType;   //报文类型，F1H（远程图像数据上送结束标记报）
    u_int8 frameNo;      //帧序列号，80H（主动上传最高位为1）
    u_int8 channelNo;    //通道号，1或2
    u_int32 timeStamp;   //本图像拍摄时间
    char MD5[32]={0};    //文件MD5码
    char reserve[8]={0}; //前1字节表示文件类型：0 图片，1 视频。后7字节备用
    u_int16 CRC16;       //CRC16校验
    u_int8 End;          //报文尾
};

/**
 * @brief 计算结构体成员的大小
 * 
 * @return int 
 */
int main() {
    ProtocolHeader_heartbeat header;

    // 获取第二个成员到第五个成员的偏移量
    size_t offsetSecond = offsetof(ProtocolB37, sync);
    size_t offsetFifth = offsetof(ProtocolB37, End);

    // 计算偏移量差值，即为第二个到第五个成员的大小
    size_t size = offsetFifth - offsetSecond;

    std::cout << "Size from all  member: " << size + 1 << std::endl;

    return 0;
}
