#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#define SIZE 500
#define BUF_SIZE 10
#define CLIENT_LIMIT 10

struct stack_char {
  char STACK_CHAR[SIZE];
  int top;
};


void initialise(struct stack_char *s) {
  s->top = -1;
}


int isfull(struct stack_char *s) {
  if (s->top == SIZE - 1)
    return 1;
  else
    return 0;
}


int isempty(struct stack_char *s) {
  if (s->top == -1)
    return 1;
  else
    return 0;
}


void push(struct stack_char *s, char data) {
  if (isfull(s)) {
    printf("FULL\n");
  } else {
    s->top++;
    s->STACK_CHAR[s->top] = data;
  }

}


void pop(struct stack_char *s) {
  if (isempty(s)) {
    printf("EMPTY \n");
  } else {
    s->top--;
  }
}

char peek(struct stack_char *s){
  if(s->top==-1) return '\0';
  return s->STACK_CHAR[s->top];
}


int precedence (char c) {
	if (c == '/' || c == '*') {
		return 2;
    }
	else if (c == '+' || c == '-') {
		return 1;
    }
	else {
        return 0;
    }
}


int isOperand (char c) {
	return ((c >= '0' && c <= '9')||c=='.');
}


char* infixToPostfix (char buf[], int n) {
	struct stack_char* s = (struct stack_char*)malloc(sizeof(struct stack_char)); 
	initialise(s);
	char* postFix = (char*)malloc(SIZE*sizeof(char));
	int index = 0;

	for(int i = 0; i < n; i++) {
    if(buf[i]==' ') postFix[index++] = ' ';
    else if(buf[i]=='\n') continue;
    else if(buf[i]=='@') break;
		else if (isOperand(buf[i])) {
            postFix[index++] = buf[i];
        }
		else if (buf[i] == '(') {
			      push(s,buf[i]);
        }
		else if (buf[i] == ')') {
			while(peek(s) != '(') {
				postFix[index++] = peek(s);
				pop(s);
			}
			pop(s);
		}
		else {
      postFix[index++] = ' ';
			while (!isempty(s) && precedence(buf[i]) <= precedence(peek(s))) {
				postFix[index++] = peek(s);
				pop(s);
			}
			push(s,buf[i]);
		}
	}

	while (!isempty(s)) {
		postFix[index++] = peek(s);
    pop(s);
	}

	return postFix;
}


struct stack_float {
  float STACK_FLOAT[SIZE];
  int top;
};

void initialise_stack_float(struct stack_float *s) {
  s->top = -1;
}


int isfull_(struct stack_float *s) {
  return (s->top == SIZE - 1);
}


int isempty_(struct stack_float *s) {
  return (s->top == -1);
}


void push_(struct stack_float *s, float data) {
  if (isfull_(s)) {
    printf("FULL\n");
  } else {
    s->top++;
    s->STACK_FLOAT[s->top] = data;
  }
}


void pop_(struct stack_float *s) {
  if (isempty_(s)) {
    printf("EMPTY\n");
  } else {

    s->top--;
  }
}

float peek_(struct stack_float *s){
  if(s->top==-1) return 0;
  return s->STACK_FLOAT[s->top];
}

float evaluatePostfix(char* exp)
{

    struct stack_float* stack = (struct stack_float*)malloc(sizeof(struct stack_float));
    initialise_stack_float(stack);

    int i=0;
 
    while (exp[i]!='\0'&&exp[i]!='@')
    {
      if(exp[i]==' '||exp[i]=='\n'){
       i++;
       continue;

      }
        float point = 0,temp=0;
        if(isOperand(exp[i])){

        
          while (isOperand(exp[i])){
            // printf("%f ",temp);
              if(exp[i]=='.') point = 10;
              else{
                if(point){
                  temp  += (exp[i]-'0')*1.0/point;
                  point *=10;
                }else{
                  temp = 10*temp + (exp[i]-'0'); 
                }
              }
              i++;
          }
          push_(stack, temp);
          }else
          {
              float val1 = peek_(stack);
              pop_(stack);
              float val2 = peek_(stack);
              pop_(stack);
              switch (exp[i])
              {
              case '+': push_(stack, val2 + val1); break;
              case '-': push_(stack, val2 - val1); break;
              case '*': push_(stack, val2 * val1); break;
              case '/': push_(stack, val2/val1); break;
              }
              i++;
        }
    }

    return peek_(stack);
}


float computeExpression(char buf[]){
    char* temp = infixToPostfix(buf,strlen(buf));
    // printf("%s\n",temp);
    float ans = evaluatePostfix(temp);
    free(temp);
    return ans;
}

int main(){
    int sockfd,newsockfd;
    int clilen;

    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        perror("Unable create the socket\n");
        exit(0);
    }

    struct sockaddr_in client_address,server_address;
    int i,temp,flag;
    char buf[50],req[SIZE];

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(20000);

    if(bind(sockfd,(struct sockaddr*) &server_address,sizeof(server_address))<0)
    {
        perror("Unable to bind to the local address\n");
        exit(0);
    }
    listen(sockfd,CLIENT_LIMIT);

    while(1){
        flag = 0;
        clilen = sizeof(client_address);
        newsockfd = accept(sockfd,(struct sockaddr*) &client_address, &clilen);

        if(newsockfd<0){
            perror("Accept error\n");
            exit(0);
        }
        printf("Connected to a client\n");
       bzero(buf,50);
        while(1){
            bzero(req,SIZE);
            bzero(buf,50);
        
          while((temp = recv(newsockfd,buf,BUF_SIZE,0))>0){
            strcat(req,buf);
            // printf("%s\t%d\n",buf,temp);
            i = 0;
            while(i<temp){
              if(buf[i]=='@'){
                flag = 1;
                break;
              }
              // printf("%c ",buf[i]);
              i++;
            }
            // printf("------------\n");
            if(flag) break;
            if(temp==0) break;
            while(temp>0&&temp--){
              buf[temp]='\0';
            }
            buf[temp] = '\0';
          }
          if(temp>0){

          flag = 0;
          // printf("I AM ALSO FREE\n");
        printf("%s\n",req);
        // gcvt(computeExpression(req),7,buf);
        sprintf(buf,"%f",computeExpression(req));
        strcat(buf,"@");
        printf("ANSWER IS %s\n",buf);
        send(newsockfd,buf,50,0);

          }else{
            break;
          }
        }
        printf("Client disconnected\n");
        close(newsockfd);
    }

    return 0;
}
// cliejnt 100MB @ 1MB 