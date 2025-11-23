// ev_client.c
// Simple user client to talk to kernel_sim_server via UNIX domain socket.
// Compile: gcc -O2 ev_client.c -o ev_client

#include <arpa/inet.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/* Socket path must match server */
#define SOCK_PATH "/tmp/inputsim.sock"

/* Message header */
struct MsgHeader { uint32_t type; uint32_t len; };

/* Message types (match server) */
#define MT_OPEN    1
#define MT_CLOSE   2
#define MT_IOCTL   3
#define MT_OPEN_R  100
#define MT_IOCTL_R 101
#define MT_EVENT   110
#define MT_SHUTDOWN 111

/* Ioctl IDs */
#define IO_GET_NAME 1
#define IO_GET_BITS 2
#define IO_SET_FILTER 3

/* Event types (match server) */
#define EVT_SYN 0
#define EVT_KEY 1
#define EVT_ABS 3

/* Utility I/O helpers */
static int write_all(int fd, const void* buf, size_t len) {
    const char* p = (const char*)buf;
    while (len > 0) {
        ssize_t n = write(fd, p, len);
        if (n < 0) {
            if (errno == EINTR) continue;
            return 0;
        }
        p += n; len -= n;
    }
    return 1;
}
static int read_all(int fd, void* buf, size_t len) {
    char* p = (char*)buf;
    while (len > 0) {
        ssize_t n = read(fd, p, len);
        if (n <= 0) {
            if (n < 0 && errno == EINTR) continue;
            return 0;
        }
        p += n; len -= n;
    }
    return 1;
}

/* Portable 64-bit host<->big-endian conversions */
static uint64_t htobe64_u64(uint64_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((uint64_t)htonl((uint32_t)(x & 0xFFFFFFFF)) << 32) | htonl((uint32_t)(x >> 32));
#else
    return x;
#endif
}
static uint64_t be64toh_u64(uint64_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint32_t hi = ntohl((uint32_t)(x >> 32));
    uint32_t lo = ntohl((uint32_t)(x & 0xFFFFFFFF));
    return ((uint64_t)lo << 32) | hi;
#else
    return x;
#endif
}

/* small helpers */
static uint32_t hton32(uint32_t x){ return htonl(x); }

int main(int argc, char** argv) {
    const char *client_name = "clientX";
    if (argc >= 2) client_name = argv[1];

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    struct sockaddr_un addr;
    memset(&addr,0,sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path)-1);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); close(fd); return 1; }

    /* send OPEN payload: "kbd0\0clientName\0" */
    const char *devname = "kbd0";
    size_t p_len = strlen(devname) + 1 + strlen(client_name) + 1;
    char *payload = malloc(p_len);
    memcpy(payload, devname, strlen(devname)+1);
    memcpy(payload + strlen(devname)+1, client_name, strlen(client_name)+1);

    struct MsgHeader h;
    h.type = hton32(MT_OPEN);
    h.len = hton32((uint32_t)p_len);
    write_all(fd, &h, sizeof(h));
    write_all(fd, payload, p_len);
    free(payload);

    /* read open reply */
    struct MsgHeader rh;
    if (!read_all(fd, &rh, sizeof(rh))) { printf("no reply\n"); close(fd); return 1; }
    uint32_t rtype = ntohl(rh.type), rlen = ntohl(rh.len);
    if (rtype != MT_OPEN_R) { printf("unexpected reply type %u\n", rtype); close(fd); return 1; }
    if (rlen >= 4) {
        uint32_t status;
        read_all(fd, &status, 4);
        status = ntohl(status);
        if (status != 0) { printf("open failed status=%u\n", status); close(fd); return 1; }
    }

    printf("[client] opened device %s as %s\n", devname, client_name);

    /* send IOCTL GET_NAME */
    uint32_t iid = htonl(IO_GET_NAME);
    struct MsgHeader ih;
    ih.type = hton32(MT_IOCTL);
    ih.len = hton32(4);
    write_all(fd, &ih, sizeof(ih));
    write_all(fd, &iid, 4);

    /* read ioctl reply */
    if (!read_all(fd, &rh, sizeof(rh))) { close(fd); return 1; }
    rtype = ntohl(rh.type); rlen = ntohl(rh.len);
    if (rtype == MT_IOCTL_R) {
        uint32_t status;
        read_all(fd, &status, 4);
        status = ntohl(status);
        if (status == 0) {
            if (rlen > 4) {
                char *buf = malloc(rlen - 4);
                read_all(fd, buf, rlen - 4);
                printf("[client] ioctl GET_NAME -> %s\n", buf);
                free(buf);
            }
        } else {
            printf("[client] ioctl GET_NAME failed\n");
        }
    }

    /* IOCTL GET_BITS */
    iid = htonl(IO_GET_BITS);
    ih.type = hton32(MT_IOCTL); ih.len = hton32(4);
    write_all(fd, &ih, sizeof(ih)); write_all(fd, &iid, 4);
    if (!read_all(fd, &rh, sizeof(rh))) { close(fd); return 1; }
    rtype = ntohl(rh.type); rlen = ntohl(rh.len);
    if (rtype == MT_IOCTL_R) {
        uint32_t status;
        read_all(fd, &status, 4); status = ntohl(status);
        if (status == 0) {
            if (rlen >= 8) {
                uint32_t mask_net;
                read_all(fd, &mask_net, 4);
                uint32_t mask = ntohl(mask_net);
                printf("[client] ioctl GET_BITS -> mask=0x%08x\n", mask);
            }
        }
    }

    /* IOCTL set filter to only EVT_KEY (bit EVT_KEY) */
    uint32_t setid = htonl(IO_SET_FILTER);
    uint32_t mask = htonl(1u << EVT_KEY);
    ih.type = hton32(MT_IOCTL); ih.len = hton32(8);
    write_all(fd, &ih, sizeof(ih));
    write_all(fd, &setid, 4);
    write_all(fd, &mask, 4);
    if (!read_all(fd, &rh, sizeof(rh))) { close(fd); return 1; }
    rtype = ntohl(rh.type); rlen = ntohl(rh.len);
    if (rtype == MT_IOCTL_R) {
        uint32_t status;
        read_all(fd, &status, 4); status = ntohl(status);
        printf("[client] set filter status=%u\n", status);
    }

    printf("[client] enter read loop; press Ctrl-C to exit\n");
    /* read loop: read framed messages */
    while (1) {
        struct MsgHeader mh;
        if (!read_all(fd, &mh, sizeof(mh))) break;
        uint32_t mtype = ntohl(mh.type), mlen = ntohl(mh.len);
        if (mtype == MT_EVENT) {
            if (mlen != (sizeof(uint64_t)+4+4+4)) {
                /* unexpected */
                if (mlen>0) {
                    char *tmp = malloc(mlen);
                    read_all(fd, tmp, mlen);
                    free(tmp);
                }
                continue;
            }
            uint64_t ms_net; uint32_t t_net, c_net; int32_t v_net;
            read_all(fd, &ms_net, 8);
            read_all(fd, &t_net, 4);
            read_all(fd, &c_net, 4);
            read_all(fd, &v_net, 4);
            uint64_t ms = be64toh_u64(ms_net);
            uint32_t t = ntohl(t_net);
            uint32_t c = ntohl(c_net);
            int32_t v = ntohl(v_net);
            printf("[client] EVENT t=%" PRIu64 " type=%u code=%u val=%d\n", ms, t, c, v);
        } else if (mtype == MT_SHUTDOWN) {
            printf("[client] server shutdown\n"); break;
        } else {
            /* skip payload */
            if (mlen>0) {
                char *tmp = malloc(mlen);
                read_all(fd, tmp, mlen);
                free(tmp);
            }
        }
    }

    close(fd);
    return 0;
}

