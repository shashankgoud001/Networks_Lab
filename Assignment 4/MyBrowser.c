#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>

#define URL_MAX_LENGTH 1024
#define HEADER_SIZE 4096

// removes the leading and trailing spaces
void trim_url(char *url)
{
    int l = strlen(url);
    int i = 0, j = l - 1;
    while (i < l && url[i] == ' ')
        i++;
    while (j >= 0 && url[j] == ' ' || url[j] == '\n')
        j--;
    char tr_url[URL_MAX_LENGTH];
    int k = 0;
    for (k = 0; k <= j - i; k++)
    {
        tr_url[k] = url[k + i];
    }
    tr_url[k] = '\0';
    strcpy(url, tr_url);
}

// parses the url and returns the host, path and port
int parse_url(char *url, char *host, char *path, int *port)
{
    int len = strlen(url);
    if (strncmp(url, "http://", 7) == 0)
    {
        url += 7;
        char *h = strchr(url, '/');
        char *p = strchr(url, ':');
        if (h != NULL)
        {
            int i = 0;
            while (i < len && url[i] != '/')
            {
                host[i] = url[i];
                i++;
            }
            host[i] = '\0';
            int j = 0;
            while (i < len && url[i] != '\0' && url[i] != ':' && url[i] != ' ')
            {
                path[j] = url[i];
                i++;
                j++;
            }
            path[j] = '\0';
            if (url[i] == ':')
            {
                i++;
                *port = 0;
                while (i < len && url[i] != '\0' && url[i] != '/' && url[i] != ' ')
                {
                    *port = *port * 10 + (url[i] - '0');
                    i++;
                }
            }
            else
            {
                *port = 80;
            }
        }
        else if (p != NULL)
        {
            int i = 0;
            while (i < len && url[i] != ':')
            {
                host[i] = url[i];
                i++;
            }
            host[i] = '\0';
            i++;
            *port = 0;
            while (i < len && url[i] != '\0' && url[i] != '/' && url[i] != ' ')
            {
                *port = *port * 10 + (url[i] - '0');
                i++;
            }
            strcpy(path, "/");
        }
        else
        {
            strcpy(host, url);
            strcpy(path, "/");
            *port = 80;
        }
        return 1;
    }
    else
    {
        printf("Invalid URL\n");
        return 0;
    }
}

// sends the http request to the server
void send_expression(int sockfd, char *expression)
{
    int len = strlen(expression);
    int i = 0, pos = 0;
    char buf[100];
    while (pos < len)
    {
        bzero(buf, 100);
        while (i < 100 && pos < len)
        {
            buf[i] = expression[pos];
            i++;
            pos++;
        }
        if (i == 100)
        {
            send(sockfd, buf, 100, 0);
            i = 0;
        }
        if (pos == len)
        {
            if (i > 0)
            {
                buf[i] = '\0';
                send(sockfd, buf, i, 0);
            }
            else
                send(sockfd, "", 1, 0);
        }
    }
}

// receives the response from the server
void recv_expression(int sockfd, char *f_name, char *opener)
{
    int nw_ln = 0, pr_fl = 0, nl = 0, wrt = 0, frst_ln = 1;
    char first_line[100];
    int pos = 0, done = 0, opnd = 0;

    FILE *fptr = NULL;
    char buf[100];
    int it = 2;
    while (1)
    {
        bzero(buf, 100);

        int n = recv(sockfd, buf, 100, 0);
        if (n <= 0)
            break;

        for (int i = 0; i < n; i++)
        {
            if (nw_ln && buf[i] == '\r')
            {
                pr_fl = 1;
            }
            else
            {
                nw_ln = 0;
            }
            if (buf[i] == '\n')
            {
                nw_ln = 1;
                frst_ln = 0;
            }
            if (pr_fl)
            {
                if (nl >= 2)
                {
                    if (!opnd)
                    {
                        printf("Opening %s\n", f_name);
                        fptr = fopen(f_name, "wb");
                        if (fptr == NULL)
                        {
                            printf("Error opening file\n");
                            return;
                        }
                        opnd = 1;
                    }
                    fwrite(&buf[i], 1, 1, fptr);
                    wrt = 1;
                }
                else
                    nl++;
            }
            else
            {
                printf("%c", buf[i]);
                if (frst_ln)
                {
                    first_line[pos++] = buf[i];
                }
                else if (!done)
                {
                    char *pr = strtok(first_line, " ");
                    pr = strtok(NULL, " ");
                    if (strcmp(pr, "200") != 0)
                    {
                        f_name = "error.html";
                        opener = "firefox";
                    }
                    done = 1;
                }
            }
        }
    }
    if (fptr)
        fclose(fptr);

    if (wrt && fork() == 0)
    {
        char *argv[] = {opener, f_name, NULL};
        execvp(argv[0], argv);
    }
}

int get_file_size(char *f_name)
{
    FILE *fptr = fopen(f_name, "rb");
    if (fptr == NULL)
    {
        printf("Error opening file\n");
        return -1;
    }
    fseek(fptr, 0, SEEK_END);
    int size = ftell(fptr);
    fclose(fptr);
    return size;
}

void send_file(int sockfd, char *fname)
{
    int size = get_file_size(fname);
    char buf[100];
    FILE *fptr = fopen(fname, "rb");
    if (fptr == NULL)
    {
        printf("Error opening file\n");
        return;
    }
    int pos = 0;
    while (pos < size)
    {
        bzero(buf, 100);
        int n = fread(buf, 1, 100, fptr);
        pos += n;
        send(sockfd, buf, n, 0);
    }
    fclose(fptr);
}

int main()
{
    int sockfd;
    struct sockaddr_in serv_addr;

    char header[HEADER_SIZE];
    char url[URL_MAX_LENGTH];
    char host[1024];
    char path[1024];
    int port;

    while (1)
    {
        printf("MyBrowser> ");
        fgets(url, URL_MAX_LENGTH, stdin);
        trim_url(url);
        // printf("URL: %s\n", url);
        if (strncmp(url, "QUIT", 4) == 0)
            break;

        if (strncmp(url, "GET", 3) == 0)
        {
            char *s = url + 4;
            trim_url(s);
            int ret = parse_url(s, host, path, &port);
            if (!ret)
                continue;

            // printf("Host: %s\n", host);
            // printf("Path: %s\n", path);
            // printf("Port: %d\n", port);

            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("[-] Error in creating a socket\n");
                exit(0);
            }

            serv_addr.sin_family = AF_INET;
            inet_aton(host, &serv_addr.sin_addr);
            // inet_aton("203.110.245.250", &serv_addr.sin_addr);
            serv_addr.sin_port = htons(port);

            // printf("connected\n");
            sprintf(header, "GET %s HTTP/1.1\r\n", path);

            strcat(header, "Host: ");
            strcat(header, host);
            strcat(header, "\r\n");

            // strcat(header, "User-Agent: Mozilla/98.0.1\n");

            int l = strlen(path);
            char *ext = path + l;

            // getting the file extension
            while (ext != path && *ext != '.')
                ext--;
            ext++;

            char opener[10];
            // finding the file type
            if (strcmp(ext, "html") == 0)
            {
                strcat(header, "Accept: text/html\r\n");
                strcpy(opener, "firefox");
            }
            else if (strcmp(ext, "jpg") == 0)
            {
                strcat(header, "Accept: image/jpeg\r\n");
                strcpy(opener, "open");
            }
            // else if (strcmp(ext, "png") == 0)
            // {
            //     strcat(header, "Accept: image/png\r\n");
            //     strcpy(opener, "open");
            // }
            else if (strcmp(ext, "pdf") == 0)
            {
                strcat(header, "Accept: application/pdf\r\n");
                strcpy(opener, "open");
            }
            else
            {
                strcat(header, "Accept: text/*\r\n");
                strcpy(opener, "gedit");
            }
            strcat(header, "Accept-Language: en-US,en;q=0.8\r\n");

            strcat(header, "If-Modified-Since: ");

            time_t now;
            struct tm *time_info;

            time(&now);
            time_info = localtime(&now);
            time_info->tm_mday -= 2;
            mktime(time_info);

            char *lst_modifd = asctime(time_info);
            l = strlen(lst_modifd);
            lst_modifd[l - 1] = '\0';

            strcat(header, lst_modifd);
            strcat(header, "\r\n");
            
            strcat(header, "Connection: close\r\n\r\n");

            printf("%s", header);
            
            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                printf("[-] Error in connecting to the server\n");
                continue;
            }

            send_expression(sockfd, header);

            // getting the file name
            ext = path + l;
            while (ext != path && *ext != '/')
                ext--;

            ext++;

            struct pollfd count_down;
            count_down.fd = sockfd;
            count_down.events = POLLIN;

            if (poll(&count_down, 1, 3000) == 0) // wait for 3 seconds for the response
            {
                printf("[-] Timed OUT\n");
                close(sockfd);
                continue;
            }

            recv_expression(sockfd, ext, opener);
            close(sockfd);
        }
        else if (strncmp(url, "PUT", 3) == 0)
        {
            char *s = url + 4;
            trim_url(s);
            char *file_name = s;
            int j = strlen(s);
            while (j >= 0 && s[j] != ' ')
                j--;
            file_name = s + j + 1;
            s[j] = '\0';

            char *actual_file_name = file_name + strlen(file_name);
            while (actual_file_name != file_name && *actual_file_name != '/')
                actual_file_name--;
            
            if (*actual_file_name == '/')
                actual_file_name++;

            trim_url(s);
            int ret = parse_url(s, host, path, &port);
            if (!ret)
                continue;

            // printf("Host: %s\n", host);
            // printf("Path: %s\n", path);
            // printf("Port: %d\n", port);
            // printf("File: %s\n", file_name);
            j = strlen(path);
            if (path[j - 1] != '/')
                strcat(path, "/");

            strcat(path, actual_file_name);

            sprintf(header, "PUT %s HTTP/1.1\r\n", path);
            strcat(header, "Host: ");
            strcat(header, host);
            strcat(header, "\r\n");

            int l = strlen(file_name);
            char *ext = file_name + l;

            // getting the file extension
            while (ext != path && *ext != '.')
                ext--;
            ext++;

            char opener[10];
            // finding the file type
            if (strcmp(ext, "html") == 0)
            {
                strcat(header, "Content-type: text/html\r\n");
                strcpy(opener, "firefox");
            }
            else if (strcmp(ext, "jpg") == 0)
            {
                strcat(header, "Content-type: image/jpeg\r\n");
                strcpy(opener, "open");
            }
            // else if (strcmp(ext, "png") == 0)
            // {
            //     strcat(header, "Content-type: image/png\r\n");
            //     strcpy(opener, "open");
            // }
            else if (strcmp(ext, "pdf") == 0)
            {
                strcat(header, "Content-type: application/pdf\r\n");
                strcpy(opener, "open");
            }
            else
            {
                strcat(header, "Content-type: text/*\r\n");
                strcpy(opener, "gedit");
            }
            // strcat(header, "Accept-Language: en-US,en;q=0.8\r\n");
            char size[50];
            long long int sz = get_file_size(file_name);
            if (sz == -1)
                continue;
            sprintf(size, "Content-length: %lld\r\n", sz);
            strcat(header, size);

            strcat(header, "If-Modified-Since: ");

            time_t now;
            struct tm *time_info;

            time(&now);
            time_info = localtime(&now);
            time_info->tm_mday -= 2;
            mktime(time_info);

            char *lst_modifd = asctime(time_info);
            l = strlen(lst_modifd);
            lst_modifd[l - 1] = '\0';

            strcat(header, lst_modifd);
            strcat(header, "\r\n");

            strcat(header, "Connection: close\r\n\r\n");

            printf("%s", header);

            if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            {
                perror("[-] Error in creating a socket\n");
                exit(0);
            }

            serv_addr.sin_family = AF_INET;
            inet_aton(host, &serv_addr.sin_addr);
            // inet_aton("203.110.245.250", &serv_addr.sin_addr);
            serv_addr.sin_port = htons(port);

            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                printf("[-] Error in connecting to the server\n");
                continue;
            }

            send_expression(sockfd, header);
            send_file(sockfd, file_name);
            // print_response(sockfd);

            struct pollfd count_down;
            count_down.fd = sockfd;
            count_down.events = POLLIN;
            
            if (poll(&count_down, 1, 3000) == 0) // wait for 3 seconds for the response
            {
                printf("[-] Timed OUT\n");
                close(sockfd);
                continue;
            }
            recv_expression(sockfd, "error.html", "firefox");
            close(sockfd);
        }
        else
        {
            printf("Invalid command\n");
            continue;
        }
        bzero(header, HEADER_SIZE);
        bzero(url, URL_MAX_LENGTH);
        bzero(host, 1024);
        bzero(path, 1024);
    }

    printf("Bye!\n");
    return 0;
}