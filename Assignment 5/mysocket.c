#include "mysocket.h"

#define S_SLEEP_TIME 1
#define MSG_MAX_SIZE 1000
#define MAX_RECV_MSG_SIZE 5000

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct
{
    char *message;
    int len;
    int flags;
} MESSAGE;

int send_counter;
int recv_counter;
int recv_idx;
int bytes_recv;
int main_sockfd;
int current_sockfd;
int send_flag;

MESSAGE send_message_buffer[10];
MESSAGE recv_message_buffer[10];

pthread_t send_thread;
pthread_t recv_thread;
pthread_mutex_t send_message_table_lock;
pthread_mutex_t recv_message_table_lock;

void *send_message_S(void *param)
{

    while (1)
    {
        sleep(S_SLEEP_TIME);

        pthread_mutex_lock(&send_message_table_lock);

        for (int i = 0; i < send_counter; ++i)
        {
            send(current_sockfd, send_message_buffer[i].message, send_message_buffer[i].len, send_message_buffer[i].flags);
            send_message_buffer[i].len = 0;
            send_message_buffer[i].flags = 0;
        }
        send_counter = 0;
        send_flag = 0;
        pthread_mutex_unlock(&send_message_table_lock);
    }
}
void *recv_message_R(void *param)
{

    char *buf = (char *)malloc(MAX_RECV_MSG_SIZE * (sizeof(char)));
    int len, l, j, n, m, prev_len;
    while (1)
    {
        while (current_sockfd == -1)
            ;
        int len_message = 0;

        int i = 0;
        while (1)
        {
            int k = recv(current_sockfd, buf, 1, 0);
            if (k <= 0)
            {
                current_sockfd = -1;
                break;
            }
            if (buf[0] == '\n')
            {
                break;
            }
            len_message = len_message * 10 + (buf[0] - '0');
        }
        if (current_sockfd == -1)
            continue;

        pthread_mutex_lock(&recv_message_table_lock);

        while (len_message > 0)
        {
            l = recv(current_sockfd, buf, len_message, 0);
            if (l <= 0)
                break;
            int flag = 1;

            while (recv_counter == 10)
            {
                pthread_mutex_unlock(&recv_message_table_lock);
                sleep(1);
                pthread_mutex_lock(&recv_message_table_lock);
            }

            j = 0;
            prev_len = recv_message_buffer[(recv_counter + recv_idx) % 10].len;
            while (j < l)
            {
                recv_message_buffer[(recv_counter + recv_idx) % 10].message[j + prev_len] = buf[j];
                j++;
            }
            recv_message_buffer[(recv_counter + recv_idx) % 10].len = prev_len + l;

            len_message -= l;

            if (len_message == 0)
                recv_counter++;
        }

        pthread_mutex_unlock(&recv_message_table_lock);
    }
}

int my_socket(int domain, int type, int protocol)
{
    if (type != SOCK_MyTCP)
    {
        perror("my_socket: type error");
        exit(1);
    }

    int sockfd;
    sockfd = socket(domain, SOCK_STREAM, protocol);
    main_sockfd = sockfd;
    if (sockfd < 0)
    {
        perror("my_socket: socket error");
        exit(1);
    }

    for (int i = 0; i < 10; ++i)
    {
        send_message_buffer[i].message = (char *)malloc(MSG_MAX_SIZE * sizeof(char));
        send_message_buffer[i].len = 0;
        send_message_buffer[i].flags = 0;
    }
    for (int i = 0; i < 10; ++i)
    {
        recv_message_buffer[i].message = (char *)malloc(MAX_RECV_MSG_SIZE * sizeof(char));
        recv_message_buffer[i].len = 0;
        recv_message_buffer[i].flags = 0;
    }
    send_counter = 0;
    recv_counter = 0;
    bytes_recv = 0;
    current_sockfd = -1;

    pthread_attr_t send_attr, recv_attr;
    pthread_attr_init(&send_attr);
    pthread_attr_init(&recv_attr);

    pthread_mutex_init(&send_message_table_lock, NULL);
    pthread_mutex_init(&recv_message_table_lock, NULL);

    pthread_create(&(send_thread), &send_attr, send_message_S, NULL);
    pthread_create(&(recv_thread), &recv_attr, recv_message_R, NULL);
    return sockfd;
}

int my_bind(int sockfd, struct sockaddr *addr, int addrlen)
{
    return bind(sockfd, addr, addrlen);
}

int my_listen(int sockfd, int backlog)
{
    return listen(sockfd, backlog);
}

int my_accept(int sockfd, struct sockaddr *addr, int *addrlen)
{
    current_sockfd = accept(sockfd, addr, addrlen);
    int new_sockfd;
    new_sockfd = current_sockfd;
    return new_sockfd;
}

int my_connect(int sockfd, struct sockaddr *addr, int addrlen)
{
    current_sockfd = sockfd;
    return connect(sockfd, addr, addrlen);
}

int my_send(int sockfd, void *buf, int len, int flags)
{
    int j, n, m = 0, temp = len;
    char *message = (char *)buf;

    while (len > 0)
    {

        pthread_mutex_lock(&send_message_table_lock);
        while (send_counter == 10)
        {
            pthread_mutex_unlock(&send_message_table_lock);
            sleep(1);
            pthread_mutex_lock(&send_message_table_lock);
        }
        char buf2[50];
        sprintf(buf2, "%d\n", len);
        int l = strlen(buf2);
        for (int i = 0; i < l; i++)
        {
            send_message_buffer[0].message[i] = buf2[i];
        }
        send_message_buffer[0].len = l;
        send_message_buffer[0].flags = flags;
        send_counter++;

        for (int i = 1; i < 10 && len > 0; ++i)
        {
            j = 0;
            n = min(len, 1000);
            while (j < n)
            {
                send_message_buffer[i].message[j] = message[(i - 1) * 1000 + j];
                j++;
            }
            len -= n;
            send_message_buffer[i].len = n;
            send_message_buffer[i].flags = flags;
            send_counter++;
        }

        pthread_mutex_unlock(&send_message_table_lock);
    }
    return temp;
}

int my_recv(int sockfd, void *buf, int len, int flags)
{
    int j = 0, n = len;
    char *message = (char *)buf;

    pthread_mutex_lock(&recv_message_table_lock);

    while (recv_counter == 0)
    {
        pthread_mutex_unlock(&recv_message_table_lock);
        sleep(1);
        pthread_mutex_lock(&recv_message_table_lock);
    }

    while (j < len && j < recv_message_buffer[(recv_idx) % 10].len)
    {
        message[j] = recv_message_buffer[(recv_idx) % 10].message[j];
        j++;
    }
    recv_message_buffer[(recv_idx) % 10].len = 0;
    recv_counter--;
    recv_idx++;

    pthread_mutex_unlock(&recv_message_table_lock);
}

int my_close(int sockfd)
{
    sleep(5);
    if (sockfd != main_sockfd)
    {
        return close(sockfd);
    }

    for (int i = 0; i < send_counter; ++i)
    {
        free(send_message_buffer[i].message);
        send_message_buffer[i].len = 0;
        send_message_buffer[i].flags = 0;
    }
    for (int i = 0; i < recv_counter; ++i)
    {
        free(recv_message_buffer[i].message);
        recv_message_buffer[i].len = 0;
        recv_message_buffer[i].flags = 0;
    }

    send_counter = 0;
    recv_counter = 0;
    pthread_cancel(send_thread);
    pthread_cancel(recv_thread);
    pthread_kill(send_thread, SIGKILL);
    pthread_kill(recv_thread, SIGKILL);
    return close(sockfd);
}
