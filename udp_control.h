#ifndef UDP_CONTROL_H
#define UDP_CONTROL_H

// 函数声明
void cotrl_handle(char *recvline, int ret);
void udp_control(char *recvline);
void udp_thread(void *pdata);
void start_udp_thread(void);

#endif