#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#define SIZE 5000
#define SIZE_MAX_ 1000
#define BUF_SIZE 100
#define LIMIT_ROWS_ 100

char *error_400, *error_403, *error_404, *error_304;

struct headers
{
    char *method;
    char *url;
    char *protocol_version;
    char *host;
    char *connection;
    char *Date;
    char *Accept;
    char *Accept_Language;
    char *If_Modified_Since;
    char *Content_Language;
    char *Content_Length;
    char *Content_Type;
    char *Expires;
    char *Cache_Control;
    char *Last_Modified;
    char *status_Message;
    int status;
};
void send_any_thing(int newsockfd, char buf[], char *response, int len)
{
    int l = 0, c = 0, i = 0;
    while (l < len)
    {
        for (i = 0; i < BUF_SIZE && l + i < len; ++i)
        {
            buf[i] = response[l + i];
        }
        send(newsockfd, buf, i, 0);
        l += i;
    }
}
void send_header(int newsockfd, char buf[], char response[], int len)
{
    int l = 0, c = 0, i = 0;
    while (l < len)
    {
        for (i = 0; i < BUF_SIZE && l + i < len; ++i)
        {
            buf[i] = response[l + i];
        }
        send(newsockfd, buf, i, 0);
        l += i;
    }
}
void append_date(struct headers *h, int offset)
{
    time_t a;
    a = time(NULL) + 86400 * offset;
    h->Expires = ctime(&a);
    int len = strlen(h->Expires);
    h->Expires[len - 1] = '\0';
}
int send_any_file(int newsockfd, char buf[], char filename[])
{
    bzero(buf, BUF_SIZE);
    FILE *infile;
    infile = fopen(filename, "rb");
    if (infile == NULL)
    {
        printf("Error: could not open file %s", filename);
        return 0;
    }
    int t;
    while ((t = fread(buf, 1, BUF_SIZE, infile)) > 0)
    {
        send(newsockfd, buf, t, 0);
    }
    fclose(infile);
    
    bzero(buf, BUF_SIZE);
    return 1;
}

void parse(char *request, struct headers *h)
{
    h->Accept = h->Accept_Language = h->Cache_Control = h->connection = h->Content_Language = NULL;

    h->Content_Length = h->Content_Type = h->Date = h->Expires = h->host = h->If_Modified_Since = NULL;
    h->Last_Modified = h->method = h->protocol_version = h->url = NULL;
    int status = 0;
    int r = 0;
    char *value = (char *)malloc(SIZE_MAX_ * sizeof(char));
    char *row = (char *)malloc(SIZE_MAX_ * sizeof(char));
    char *val = (char *)malloc(SIZE_MAX_ * sizeof(char));
    char **rows = (char **)malloc(LIMIT_ROWS_ * sizeof(char *));
    row = strtok(request, "\r\n");
    while (row != NULL)
    {
        rows[r++] = (char *)malloc(sizeof(char) * BUF_SIZE);
        strcpy(rows[r - 1], row);
        row = strtok(NULL, "\r\n");
    }
    for (int i = 0; i < r; ++i)
    {
        if (i == 0)
        {
            val = strtok(rows[0], " ");
            h->method = val;
            val = strtok(NULL, " ");
            h->url = val;
            val = strtok(NULL, " ");
            h->protocol_version = val;
        }
        else
        {
            val = strtok(rows[i], ": ");
            if (strcmp(val, "Host") == 0)
            {
                val = strtok(NULL, ": ");
                h->host = val;
            }
            else if (strcmp(val, "Connection") == 0)
            {
                val = strtok(NULL, ": ");
                h->connection = val;
            }
            else if (strcmp(val, "Date") == 0)
            {
                val = strtok(NULL, ": ");
                h->Date = val;
            }
            else if (strcmp(val, "Accept") == 0)
            {
                val = strtok(NULL, ": ");
                h->Accept = val;
            }
            else if (strcmp(val, "Accept-Language") == 0)
            {
                val = strtok(NULL, ": ");
                h->Accept_Language = val;
            }
            else if (strcmp(val, "If-Modified-Since") == 0)
            {
                val = strtok(NULL, ": ");
                h->If_Modified_Since = val;
            }
            else if (strcmp(val, "Content-language") == 0)
            {
                val = strtok(NULL, ": ");
                h->Content_Language = val;
            }
            else if (strcmp(val, "Content-length") == 0)
            {
                val = strtok(NULL, ": ");
                h->Content_Length = val;
            }
            else if (strcmp(val, "Content-type") == 0)
            {
                val = strtok(NULL, ": ");
                h->Content_Type = val;
            }
            else if (strcmp(val, "Expires") == 0)
            {
                val = strtok(NULL, ": ");
                h->Expires = val;
            }
            else if (strcmp(val, "Cache-control") == 0)
            {
                val = strtok(NULL, ": ");
                h->Cache_Control = val;
            }
            else if (strcmp(val, "Last-modified") == 0)
            {
                val = strtok(NULL, ": ");
                h->Last_Modified = val;
            }
        }
    }
}
void recieve_header(int newsockfd, char buf[], char response[], char extra[], int *l)
{
    int t, prev = 0, i = 0, p = 0;
    int flag = 0;
    bzero(buf, BUF_SIZE + 1);

    while ((t = recv(newsockfd, buf, BUF_SIZE, 0)) > 0)
    {
        buf[t] = '\0';

        for (i = 0; i < t; ++i)
        {
            response[prev + i] = buf[i];
            if (prev == 0 && i == 0)
            {
                continue;
            }
            else
            {
                if (response[prev + i - 1] == '\n' && response[prev + i] == '\r')
                {
                    flag = 1;
                    break;
                }
            }
        }
        if (flag)
        {
            break;
        }
        prev += t;
        bzero(buf, BUF_SIZE);
    }
    response[prev + i + 1] = '\n';
    response[prev + i + 2] = '\0';
    if (flag)
    {
        if (i < t - 1)
        {
            i += 2;
            int j = 0;
            for (j = 0; j < t && i < t; ++j, i++)
            {
                extra[j] = buf[i];    
            }
                   
            *l = j;
        }
        else
        {
            recv(newsockfd, buf, 1, 0);
            *l = 0;
        }
    }
    else
    {
        extra[0] = '\0';
    }
}
void recieve_file(int newsockfd, char filename[], char *extra, int l, char buf[], int content_length)
{
    bzero(buf, BUF_SIZE);
    int t;
    
    FILE *outfile;
    outfile = fopen(filename, "wb");
    if (l)
        fwrite(extra, sizeof(extra[0]), l, outfile);

    content_length -= l;
    while (content_length && (t = recv(newsockfd, buf, BUF_SIZE, 0)) > 0)
    {
        fwrite(buf, sizeof(buf[0]), t, outfile);
        content_length -= t;
    }

    fclose(outfile);
}
int get_file_size(char *f_name)
{
    FILE *fptr = fopen(f_name, "rb");
    fseek(fptr, 0, SEEK_END);
    int size = ftell(fptr);
    fclose(fptr);
    return size;
}
void addToAccessLog(struct headers *h,char* client_ip,int client_port){
    char day[3],month[3],year[5];
    char sec[3],min[3],hour[3];
    time_t a;
    a = time(NULL);
    struct tm b = *localtime(&a);
    sprintf(day,"%d",b.tm_mday);
    sprintf(month,"%d",b.tm_mon+1);
    sprintf(year,"%d",b.tm_year+1900);
    sprintf(hour,"%d",b.tm_hour);
    sprintf(min,"%d",b.tm_min);
    sprintf(sec,"%d",b.tm_sec);

    FILE* file = fopen("AccessLog.txt", "a");

    // If file doesn't exist, create a new one
    if (file == NULL) {
        file = fopen("AccessLog.txt", "w");
    }

   
    // printf("%d%d%d:%d%d%d:%s:%d:%s:%s\n",b.tm_mday,b.tm_mon+1,b.tm_year+1900,b.tm_hour,b.tm_min,b.tm_sec,client_ip,client_port,h->method,h->url);
    fprintf(file,"%d-%d-%d:%d-%d-%d:%s:%d:%s:%s\n",b.tm_mday,b.tm_mon+1,b.tm_year+1900,b.tm_hour,b.tm_min,b.tm_sec,client_ip,client_port,h->method,h->url);
    fclose(file);

}
void serve_client(struct headers *h, int newsockfd, char buf[],char* client_address,int client_port)
{
    char *request = (char *)malloc(SIZE * sizeof(char));
    char *response = (char *)malloc(SIZE * sizeof(char));
    char *extra = (char *)malloc((BUF_SIZE + 5) * sizeof(char));
    int l = 0;

    recieve_header(newsockfd, buf, request, extra, &l);
    printf("%s\n", request);
    
    parse(request, h);
    char *filepath = (char *)malloc(BUF_SIZE * sizeof(char));
    
    strcpy(filepath, h->url);
    printf("%d\t%s\n",client_port,client_address);
    addToAccessLog(h,client_address,client_port);
    if (strcmp(h->method, "GET") == 0)
    {
        if (access(filepath, F_OK) == -1)
        {
            h->status = 404;
            h->status_Message = "NOT FOUND";
            append_date(h, 3);
            h->Content_Type = "text/html";
            int file_size = strlen(error_404);
            sprintf(response, "%s %d %s\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", h->protocol_version, h->status, h->status_Message, h->Expires, file_size, h->Content_Type);
            printf("%s", response);
            send_header(newsockfd, buf, response, strlen(response));
            send_any_thing(newsockfd, buf, error_404, strlen(error_404));
        }
        else
        {
            if (access(filepath, R_OK) == -1)
            {
                h->status = 403;
                h->status_Message = "Forbidden";
                append_date(h, 3);
                h->Content_Type = "text/html";
                int file_size = strlen(error_403);
                sprintf(response, "%s %d %s\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", h->protocol_version, h->status, h->status_Message, h->Expires, file_size, h->Content_Type);
                printf("%s", response);
                send_header(newsockfd, buf, response, strlen(response));
                send_any_thing(newsockfd, buf, error_403, strlen(error_403));
            }
            else
            {
                if (h->If_Modified_Since != NULL)
                {
                    struct stat b;
                    if (!stat(filepath, &b))
                    {
                        time_t last_modified = mktime(localtime(&b.st_mtime));
                        struct tm req;
                        strptime(h->If_Modified_Since, "%a %b %d %H:%M:%S %Y", &req);
                        time_t req_time = mktime(&req);
                        double diffSecs = difftime(req_time+3600, last_modified);
                        if (diffSecs < 0)
                        {
                         	printf("diff: %lf", diffSecs);
                            h->status = 200;
                            h->status_Message = "OK";
                            append_date(h, 3);
                            sprintf(response, "%s %d %s\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", h->protocol_version, h->status, h->status_Message, h->Expires, get_file_size(filepath), h->Accept);
                            printf("%s", response);
                            send_header(newsockfd, buf, response, strlen(response));
                            send_any_file(newsockfd, buf, filepath);
                        }
                        else
                        {
                            h->status = 304;
                            h->status_Message = "Not Modified";
                            append_date(h, 3);
                            h->Content_Type = "text/html";
                            int file_size = strlen(error_304);
                            sprintf(response, "%s %d %s\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", h->protocol_version, h->status, h->status_Message, h->Expires, file_size, h->Content_Type);
                            printf("%s", response);
                            send_header(newsockfd, buf, response, strlen(response));
                            send_any_thing(newsockfd, buf, error_304, strlen(error_304));
                        }
                    }
                    else
                    {
                        printf("Cannot display the time.\n");
                    }
                }
                else
                {
                    h->status = 200;
                    h->status_Message = "OK";
                    append_date(h, 3);
                    sprintf(response, "%s %d %s\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", h->protocol_version, h->status, h->status_Message, h->Expires, get_file_size(filepath), h->Accept);
                    printf("%s", response);
                    send_header(newsockfd, buf, response, strlen(response));
                    send_any_file(newsockfd, buf, filepath);
                }
            }
        }
    }
    else if (strcmp(h->method, "PUT") == 0)
    {
        h->status = 403;
        h->status_Message = "Forbidden";
        append_date(h, 3);
        char *directory = (char *)malloc(BUF_SIZE * sizeof(char));
        int len = strlen(filepath) - 1, f = 0;
        while (len >= 0 && filepath[len] != '/')
        {
            len--;
        }
        strncpy(directory, filepath, len);
        strcat(directory, "");
        if (access(directory, F_OK) == -1 || access(directory, W_OK) == -1)
        {
            h->Content_Type = "text/html";
            int file_size = strlen(error_403);
            sprintf(response, "%s %d %s\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", h->protocol_version, h->status, h->status_Message, h->Expires, file_size, h->Content_Type);
            printf("%s", response);
            send_header(newsockfd, buf, response, strlen(response));
            send_any_thing(newsockfd, buf, error_403, strlen(error_403));
        }
        else
        {
            append_date(h, 3);
            h->status = 200;
            h->status_Message = "OK";
            sprintf(response, "%s %d %s\r\nExpires: %s\r\n\r\n", h->protocol_version, h->status, h->status_Message, h->Expires);
            printf("%s", response);
            int content_length = atoi(h->Content_Length);
            recieve_file(newsockfd, filepath, extra, l, buf, content_length);
            send_header(newsockfd, buf, response, strlen(response));
        }
    }
    else
    {
        h->status = 400;
        h->status_Message = "Bad Request";
        append_date(h, 3);
        h->Content_Type = "text/html";
        int file_size = strlen(error_400);
        sprintf(response, "%s %d %s\r\nExpires: %s\r\nCache-Control: no-store\r\nContent-language: en-us\r\nContent-length: %d\r\nContent-type: %s\r\n\r\n", h->protocol_version, h->status, h->status_Message, h->Expires, file_size, h->Content_Type);
        printf("%s", response);
        send_header(newsockfd, buf, response, strlen(response));   
        send_any_thing(newsockfd, buf, error_400, strlen(error_400));
    }
}

int main()
{
    int sockfd, newsockfd;
    int clilen;
    struct sockaddr_in cli_addr, serv_addr;
    struct headers req;
    int i;
    int client_port;
    char *client_address;
    char buf[SIZE_MAX_];
    bzero(buf, SIZE_MAX_);
    req.Expires = (char *)malloc(BUF_SIZE * sizeof(char));

    error_304 = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>304 Not Modified</title></head><body><h1>304 Not Modified</h1><p>The request resource is not modified.</p></body></html>";
    error_400 = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>400 Bad Request</title></head><body><h1>400 Bad Request </h1><p>The client has issued a malfunctioned or illegal request.</p></body></html>";
    error_403 = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1><p>You don't have permission to access this resource.</p></body></html>";
    error_404 = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>404 Resource not Found</title></head><body><h1>404 Resource not Found </h1><p>The requested resource could not be found on this server.</p></body></html>";
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Cannot create socket\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(20000);

    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        printf("Unable to bind local address\n");
        exit(0);
    }

    listen(sockfd, 5);

    while (1)
    {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
                           &clilen);

        if (newsockfd < 0)
        {
            printf("Accept error\n");
            exit(0);
        }

        if (fork() == 0)
        {
            close(sockfd);
            client_port = ntohs(cli_addr.sin_port);
            client_address = inet_ntoa(cli_addr.sin_addr);
            printf("%d\t%s\n",client_port,client_address);
            serve_client(&req, newsockfd, buf,client_address,client_port);
            close(newsockfd);
            exit(0);
        }

        close(newsockfd);
    }
    return 0;
}