#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#define NTHREADS 25
#define BUFLEN 1024
#define MAX_REQUEST 25

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int fd;
void chkType(int connfd, char *s);
char *fileExt(char *file);
void *callFileExt(void *sockets);

int main(int argc, char *argv[]) {
	
	struct sockaddr_in address;
	pthread_t thread_id[NTHREADS];
	
    int first;
    int second;
    int err;
    
	unsigned *t[MAX_REQUEST];
	
    int portno = 80;
    if (argc > 1) {
        if (atoi(argv[1])) {
            portno = atoi(argv[1]);
        }
    }
    
    printf("Port Number set at %i\n\n", portno);
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) > 0){
        printf("The socket was created, waiting for socket to bind...\n");
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portno);
    
    int check = 1;
    
    while (check == 1) {
        if (bind(fd, (struct sockaddr *) &address, sizeof(address)) == 0){
            printf("Socket binded\n");
            check = 0;
        }
    }
    
    
	//backlog allows you to limits the number of connections that queue up for server
	int backlog = 0;
	if (listen(fd, backlog) == -1) {
		// an error occurred
		printf("ERROR CODE: 00301 MAXIMUM CONNECTION CONNECTED\n");
	}
    
    for(first=0; first < MAX_REQUEST; first++)
    {
        t[first]=(unsigned *)malloc(sizeof(unsigned));
        *t[first]=first+1;
        printf("Thread %d Running\n",(first+1));
        err = pthread_create( &thread_id[first], NULL, callFileExt, t[first]);
        if(err)
        {
            printf("ERROR CODE: 00302 ERROR CREATING THREAD.\n");
        }
    }
    //
    for(second=0; second < MAX_REQUEST; second++)
    {
        pthread_join( thread_id[second], NULL);
    }
    
	close(fd);
	return 0;
}

void * callFileExt(void *thread) {
    
	//int fd = *(int*)sockets;
	unsigned wt = *((unsigned*)thread);
	char buf[BUFLEN];
	int connfd;
	struct sockaddr_in cadd;
	socklen_t caddLen = sizeof(cadd);
	while (1) {
		pthread_mutex_lock( &mutex1 );
		connfd = accept(fd, (struct sockaddr *) &cadd, &caddLen);
		pthread_mutex_unlock( &mutex1 );
		printf("Thread %d: \tworking\n", wt);
		
        if (connfd == -1) {
			printf("ERROR CODE: 00101 CONNECTION ERROR");
		}
        
		ssize_t count;
		count = read(connfd, buf, BUFLEN);
		if (count == -1) {
			printf("ERROR CODE: 00102 READING ERROR");
		} else {
            
			chkType(connfd, buf);
			printf("%s", buf);
		}
		close(connfd);
	}
}

void chkType(int connfd, char *buf) {
	char *type;
	char *html = NULL;
	FILE *fp;
	char *h = "HTTP/1.1 200 OK \r\n";
	char *s = "Server: Apache/1.3.34 (Unix) PHP/4.4.2 \r\n";
	char resp[600];
	struct stat size;
	char *content_length=NULL;
	
	char fn[200];
	char ptl[200];
    
	sscanf(buf, "%s /%s", ptl, fn);
	char * chk;
	
	if(strstr(buf, "Host: ") != NULL)
	{
		printf("%s\n", strstr(buf, "Host: "));
	} else {
        printf ("ERROR CODE : 00201 HOSTNAME NOT FOUND!");
    }
    
	chk = strchr(fn, '.');
    if (chk == NULL) {
        if (strcmp(&fn[strlen(fn) -1], "/") == 0) {
            strcat(fn, "index.html");
        } else {
            strcat(fn, "/index.html");
        }
    }
    
    if (strcmp(ptl, "GET")==0) {
        fp = fopen(fn, "rb");
        if (fp != NULL) {
            
            fp = fopen(fn, "rb");
            fstat(fileno(fp), &size);
            content_length = (char*) malloc(size.st_size *((int) sizeof(char)));
            sprintf(content_length, "Content Length: %lld \r\n", size.st_size);
            type = fileExt(fn);
            strcpy(resp, h);
            strcat(resp, s);
            strcat(resp, content_length);
            strcat(resp, type);
            printf("response: %s\n", resp);
            html = (char *) realloc(html, size.st_size);
            fread(html, size.st_size, 1, fp);
            write(connfd, resp, strlen(resp));
            write(connfd, html, size.st_size);
			
            
        } else if (strcmp(fn, "HTTP/1.1")==0) {
            fp = fopen("index.html", "rb");
            fstat(fileno(fp), &size);
            content_length = (char*) malloc(size.st_size *((int) sizeof(char)));
            sprintf(content_length, "Content Length: %lld \r\n", size.st_size);
            type = fileExt(fn);
            strcpy(resp, h);
            strcat(resp, s);
            strcat(resp, content_length);
            strcat(resp, type);
            printf("Response: %s\n", resp);
            html = (char*) realloc(html, size.st_size);
            fread(html, size.st_size, 1, fp);
            write(connfd, resp, strlen(resp));
            write(connfd, html, size.st_size);
			
            
        } else {
            type = "HTTP/1.1 404 File Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n\r\n";
            fp = fopen("404.html", "rb");
            fstat(fileno(fp), &size);
            html = (char*) realloc(html, size.st_size);
            fread(html, size.st_size, 1, fp);
            write(connfd, type, strlen(type));
            write(connfd, html, size.st_size);
        }
    } else {
        type = "HTTP/1.1 400 Bad Request\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n\r\n";
        fp = fopen("bad.html", "rb");
        fstat(fileno(fp), &size);
        html = (char*) realloc(html, size.st_size);
        fread(html, size.st_size, 1, fp);
        write(connfd, type, strlen(type));
        write(connfd, html, size.st_size);
    }
    free(html);
    free(content_length);
    fclose(fp);
}

char *fileExt(char *f) {
    
	char *fileType;
	if (strcmp(f, "HTTP/1.1")==0) {
		fileType = "Content-Type: text/html\r\n\r\n";
	} else {
		f = strtok(f, ".");
		f = strtok(NULL, ".");
        
		if (strcmp(f, "html")==0 || strcmp(f, "htm")==0) {
			fileType = "Content-Type: text/html\r\n\r\n";
		} else if (strcmp(f, "jpg")==0 || strcmp(f, "jpeg")==0) {
			fileType = "Content-Type: image/jpeg\r\n\r\n";
		} else if (strcmp(f, "gif")==0) {
			fileType = "Content-Type: image/gif\r\n\r\n";
		} else if (strcmp(f, "txt")==0) {
			fileType = "Content-Type: text/plain\r\n\r\n";
		} else if (strcmp(f, "ico")==0) {
			fileType = "Content-Type: text/html\r\n\r\n";
		}
		else {
			fileType = "Content-Type: application/octet-stream\r\n\r\n";
		}
	}
	return fileType;
}