/*
* CS 656 / Fall 2019 / Term Project
* Group: W2 / Pratyush (pr444), Shruthi (vk529), Kavya (kk624), Aman (ag2438), Joby (jb768), Alex (as77)
*/

#include <stdio.h>
#include <netinet/in.h> 
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>         /* import java.net.InetAddress; etc */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <signal.h>
/*--------------- end include/s and import/s --------------*/

/* GLOBALS: do not change these */
#define MAX 65536
char HOST[MAX] =""; /* HOST to connect to; byte[] in Java */
char URL[MAX] ="";  /* URL to get        ; byte[] in Java */
int  PORT;     /* port to listen on  */  
int HPORT = 80;   /* port to connect to */
struct in_addr PREFERRED; /* set in dns() */
struct sockaddr_in servaddr_info, cliaddr_info;
char input_value[MAX], next_line[] = "\n";
int counter = 0 , bytes = 0;
int connection_fd;
char request[MAX];
char firstline[MAX];
char PATH[MAX]="";
char half2[MAX]="";
char *half;
char replything[2024];
char serverResponse[MAX];
char no_of_bytes[128];
char buf[MAX];
char hport1[20] = "";
char preferred_ip[MAX]="";
struct sockaddr_in ip_addresses_res[10];
char buffer[MAX];
int bytesWrittenToClient;
char successRespFormat[] = "REQ: %s / RESP: (%d bytes transfered.)";
char failedRespFormat[] = "REQ: %s / RESP: ERROR %d";
char bad_get[10] = "", bad_version[10] = "";
/* parse: MUST set HOST and HPORT and URL */

//this function is way too long and needs to be broken up 
void check_http_version(){
 int frstlen = strlen(firstline);
 int h = 0, g = 0;
 h = frstlen - 3;
 while(h<frstlen){
 	bad_version[g] = firstline[h]; 
	g++; 
	h++;
 }
}

void check_bad_get(){
 int q = 0;
 while(q<4){
 	bad_get[q] = firstline[q];
	q++;
 }
}

void get_host(){
 if(strchr(firstline,'/') != NULL) {
  	half = strchr(firstline, '/');
  	half = half + 2;
	int length = strcspn(half, " ");
  	strncpy(half2, half, length);
  	half2[length] = '\0';  
 	}
  else {
  	int j = 0;
  	while(4+j < strlen(firstline)) {
		half2[j] = firstline[4+j];
		j++;
	}
	half2[j] = '\0';
  }
}

void store_firstline(char request[]){
int i = 0;
 while (i<strlen(request) && request[i] != '\r') {  //getting the first line from the whole HTTP request header
   firstline[i] = request[i];
   i++; 
 }
 firstline[i] = '\0';
}

int set_host_variable(){
	int k = 0;
	while(k<strlen(half2) && half2[k] != '/' && half2[k] !=':') {      //Setting the hostname 
  		HOST[k] = half2[k];
 	   	k++;
  	}
  	HOST[k] = '\0';
	printf("HOST: %s", HOST);
	return k;
}

int set_port(int k){  
	int l;
	if(strchr(half2, ':') != NULL && strchr(half2, '/') != NULL) {
		int z = 0;
		k++;
		while(half2[k] != '/') {
			hport1[z] = half2[k];
			k++;
			z++;
		}
		hport1[z] = '\0';
		l = strlen(HOST) + strlen(hport1) + 1;
	}
  	else if(strchr(half2, ':') != NULL) {
		int z = 0;
		k++;
		while(half2[k] != '\0') {
			hport1[z] = half2[k];
			k++;
			z++;
		}
		hport1[z] = '\0';
		l = strlen(HOST) + strlen(hport1) + 1;
	}
  	else {
		strcpy(hport1, "80");
  		l=strlen(HOST);
	}
	return l;
} 

void set_path_variable(path_start_index) {
  int start = path_start_index;
  int l = path_start_index;

  while(l<strlen(half2)) {
    	PATH[l - start] = half2[l];
    	l++;
	}
  if(l != start)
  	PATH[l-start] = '\0';
  else {
	  PATH[0] = '/';
	  PATH[1] = '\0';
	 }
	printf("PATH: %s", PATH);
}

int parse(char request[])	            /* you can define the signature for dns() */
{       
 int host_length;
 puts("ENTIRE REQUEST!");
 puts(request);
 store_firstline(request);
 puts("FIRSTLINE");
 puts(firstline);
 check_http_version();
 check_bad_get();
 get_host();
 host_length = set_host_variable();

  int path_start_index = set_port(host_length);

  set_path_variable(path_start_index);
  HPORT = atoi(hport1);                   //Setting the HPORT                
  strcpy(URL,HOST);
  strcat(URL,PATH);                       //Setting the URL
  return 0;
}

int dns(int connection_fd, char host1[], char ip_array[10][30])          //performing a dns lookup for the given hostname
{ 
  struct addrinfo hints;
  struct addrinfo *res;
  struct addrinfo *p;
  struct sockaddr_in *addr;
  int out;
  memset(&hints,0,sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int a = 0;
  if ((out = getaddrinfo(host1, NULL , &hints, &res)) !=0 )  
    { return 1;}
  for(p = res; p !=NULL; p = p->ai_next)                   
    { addr = (struct sockaddr_in *) p->ai_addr;
      strcpy(ip_array[a],inet_ntoa(addr->sin_addr));    
      a++; }
  freeaddrinfo(res);
  return a;
}

int get_preferred(char ip_addresses[10][30], int num) {                //setting the preferred_ip
	return 0;
}

int sendResponseToClient(int connfd,int status_code,char type[],char data[]){                                    //status code response to the client
  char reply_format[] = "HTTP/1.1 %s\r\nContent-Length: %d\r\nContent-Type: %s\r\nConnection: close\r\n\r\n%s";
  char status_and_text[50];
  char content_type[50];
  char reply[1024];
  switch(status_code){
    case 200: strcpy(status_and_text,"200 OK");break;
    case 404: strcpy(status_and_text,"404 NOT FOUND");break;
    case 413: strcpy(status_and_text,"413 Request Entity Too Large");break; 
    case 400: strcpy(status_and_text,"400 Bad Request");break;
    case 505: strcpy(status_and_text,"505 HTTP Version Not Supported");break;
  }
  if(strcmp(type,"text")==0||strlen(type)==0){strcpy(content_type,"text/plain");}
  if(strcmp(type,"html")==0){strcpy(content_type,"text/html");}
  int content_length = (int)strlen(data);
  sprintf(reply,reply_format,status_and_text,content_length,content_type,data);
  write(connfd,reply,sizeof(reply));
  sprintf(serverResponse,failedRespFormat,HOST,status_code);
  printf("\n%s\n\n",serverResponse);
  close(connfd);
  return 0;
 }

int returnLocalFile(int connfd){                                        //retrieving the file from local directory
  FILE *fp;
  char path[] = ".";
  strcat(path,PATH);
  char* fileName = path;
  fp = fopen(fileName, "rb");
  if (fp == NULL){
    char message[1024]; char mesasge_format[] = "Could not find file %s";
    sprintf(message,mesasge_format,path); 
    sendResponseToClient(connfd,404,"text",message);
    return 1;
  }
  fseek(fp, 0, SEEK_END);
  unsigned long filesize = ftell(fp);
  rewind(fp);
  char *buffer = (char*)malloc(sizeof(char)*filesize);
  fread(buffer, sizeof(char), filesize, fp);
  char responseHead[1024]; char responseFormat[] = "HTTP/1.1 200 OK\r\nContent-Length: %d\nConnection: close\r\n\r\n";
  sprintf(responseHead,responseFormat,filesize);
  write(connfd,responseHead,strlen(responseHead));   write(connfd,buffer,filesize);  bytesWrittenToClient = filesize;
  sprintf(serverResponse,successRespFormat,URL,bytesWrittenToClient);
  printf("\n%s\n\n",serverResponse);
  fclose(fp);
  return 0;  
}

int ht_fetch(char ip[],int connfd){                   /* ht_fetch: will go get the file OR serve it locally */
 int sfd;
 struct sockaddr_in servaddr;
 char buffer[1024];
 if((sfd = socket(AF_INET, SOCK_STREAM, 0)) == 0)   //Server Socket definition
  { perror("Socket definition failed");
    exit(EXIT_FAILURE);}
 servaddr.sin_family = AF_INET;                   
 servaddr.sin_addr.s_addr = inet_addr(ip);
 servaddr.sin_port = htons( HPORT );
  
 if (connect(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  { close(sfd);
  	perror("site connect failed");}
 int n = write(sfd,input_value,strlen(input_value));
 struct timeval timeout; 
 timeout.tv_sec = 1;
 timeout.tv_usec = 0;
 setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
 bzero(buffer,sizeof(buffer));
 int bytesWrote = 0; int q=0;
 while(( n = read(sfd,buffer,sizeof(buffer)) )>0){
  	q = write(connfd,buffer,n); bytesWrote += q; 
  	bzero(buffer, sizeof(buffer));
  }
 bytesWrittenToClient = bytesWrote;
 if(bytesWrittenToClient>0){
  sprintf(serverResponse,successRespFormat,HOST,bytesWrittenToClient);
  printf("\n%s\n\n",serverResponse);}
 close(sfd);
 return 0;
}

int handle_not_found(char* preferred_ip) {
        if(strcmp(preferred_ip,"")==0){ 
		sendResponseToClient(connection_fd,404,"text","404 PAGE NOT FOUND"); 
		return 0; 
	}
	return 1;
}

int handle_request_errors() {
	int good = 1;
	if (bytes>MAX){ 
		sendResponseToClient(connection_fd,413,"text","413 Request Entity Too Large"); 
		good = 0;  
	}             //413 Large Request
        if(strcmp(bad_get,"GET ") != 0){ 
		printf("%lu", strlen(bad_get));
		puts(bad_get);
		sendResponseToClient(connection_fd,400,"text","400 Bad Request"); 
		good = 0; 
	}//400 Bad request 
        /* if((HPORT <1024 && HPORT != 80) || HPORT>65535){
		sendResponseToClient(connection_fd,400,"text","400 Bad Request"); 
		continue;
	}  //Bad Port */
        if(strcmp(bad_version,"1.1") != 0 && strcmp(bad_version,"1.0") != 0){
		sendResponseToClient(connection_fd,505,"text","505 HTTP Version Not Supported"); 
		good = 0;
	}
	return good;
}
/* note: you MUST use the basic while(1) loop here; you can
 * add things to it but the 3 methods MUST be called in this order */
int main(int argc, char **argv) /* in Java, this is run()  */   /* init sockets etc */
{ 
  sigaction(SIGPIPE, &(struct sigaction){{SIG_IGN}}, NULL);           //used to handle interrupts that was making our server stop
  int sockfd;
  char ip_addresses[10][30];
  int addrlen = sizeof(cliaddr_info);
  struct sockaddr_in serverIP;
  counter = 0;
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
  	perror("Socket definition failed");
	exit(EXIT_FAILURE);
  } 
  PORT = atoi(argv[1]);                                 //Port assignment;
  servaddr_info.sin_family = AF_INET;                   //Specifying address for server socket;
  servaddr_info.sin_addr.s_addr = INADDR_ANY;
  servaddr_info.sin_port = htons( PORT );
  if( bind(sockfd, (struct sockaddr *)&servaddr_info, sizeof(servaddr_info)) < 0 ) { 
  	perror("Binding failed");
  	exit(EXIT_FAILURE); 
  }
  if( listen( sockfd, 220) < 0 ) { 
  	perror("Issues with listen to this socket");
	exit(EXIT_FAILURE);
  }
  else { 
  	printf("\nApache Listening on socket %d\n",PORT);
  }
    
  while(1) 
  {                    
   if( (connection_fd = accept( sockfd, (struct sockaddr *)&cliaddr_info, (socklen_t*)&addrlen )) < 0 ) {   /* accept */   
      perror("Can't Accept the connection"); 
      exit(EXIT_FAILURE); 
      }
    else { 
      	counter+=1;
	getsockname(connection_fd,(struct sockaddr *)&serverIP,(socklen_t*)&addrlen );
        printf("\n(%d) Incoming client connection from [%s",counter,inet_ntoa(cliaddr_info.sin_addr));
	printf(":%d]",(int) ntohs(cliaddr_info.sin_port));
        printf("to me [%s",inet_ntoa(serverIP.sin_addr));
	printf(":%d]\n",(int) ntohs(servaddr_info.sin_port));
        int num_addresses =0,preferred_num;
      	if((bytes= read(connection_fd, &input_value, sizeof(input_value))) < 0)
		    perror("read from website failed\n");                                      //receiving the input string from the client
         parse(input_value);                                                        //calling the parse function
 	if(handle_request_errors() == 0)
		continue;
                                                                                                          //505 Unsupported Version
        if(strcmp(HOST,"localfile")==0){ 
		returnLocalFile(connection_fd); 
		close(connection_fd);
		continue;
	}        //retrieving the local file
        num_addresses = dns(connection_fd, HOST, ip_addresses);                  //calling dns function
    	preferred_num = get_preferred(ip_addresses, num_addresses);              //getting the preferred ip address
        strcpy(preferred_ip,ip_addresses[preferred_num]);
	if(handle_not_found(preferred_ip) == 0)
		continue;
        ht_fetch(ip_addresses[preferred_num],connection_fd);                     //getting the desired webpage using ht_fetch function
        memset(&ip_addresses,0,sizeof(ip_addresses));                            //setting ip_addresses to 0
        memset(&serverResponse,0,sizeof(serverResponse));                        
      }
    close(connection_fd);
  }
  return 0;
}
