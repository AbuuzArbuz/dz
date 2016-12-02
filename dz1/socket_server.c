#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#define size 10

void die(char *msg) {
	printf("error: %s, errno: %d\n", msg, errno);
	// grep 'define' /usr/include/asm-generic/errno.h  | grep <errno>
	exit(1);
}

struct buff {
	char name[20];
	char msg[1000];
	};// structure of data which server sends to clients

int server = 0;
struct sockaddr_in serv_addr;

struct cl {
	int fd;
	pthread_t thr;
	} clients[size]; // array of working thread's id

int num; // number of client

/* threads receive messages and send them to everybody except sender */ 

void* thr_void(void* arg)
	{
		int len,i,fd;
		char name[20];
		int k = *(int *)arg;
		struct buff recv_buf,send_buf;  
                fd = clients[k].fd;

		/* receive a name*/

		len = recv(fd,name,20,0);
		if (len < 0) die("receiving a name failed");
		printf("%s, thank you for visiting us! \n",name);
		/* main cycle */
		while(1) {
			len = recv(fd, recv_buf.msg, 1000, 0);
			if (len < 0) { // receive from the author...
				die("recv failed ");
				}
			if (len == 0) break;
			//... and send to all!(except the author) 
			
			strcpy(send_buf.name,name);
			strcpy(send_buf.msg,recv_buf.msg); 
			for (i = 0; i < size; i++) {
				if ((clients[i].thr != 0)&&(i != k)) {
						if (len = send(clients[i].fd,&send_buf,1020, 0) < 0) die("can't send");
					}
				}
			printf("sent to all by %s \n",name);
		}
		printf("%s left us \n",name);
		close(fd);
		clients[k].thr = 0; // closing descriptor of client and zero corresponding thr
	};
	
int main() {

	if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		die("can't create socket");
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(5000); 

	if (bind(server, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		die("can't bind");
	}
	if (listen(server, 10) < 0) {
		die("can't listen");
	}
	
	int i,fd;
	for (i = 0; i < size; i++) clients[i].thr = 0; //thr =  null

	printf("ready to work \n");
	struct buff recv_buf;
	/* main process accepts clients */
	while(1) {
		if ((fd = accept(server, (struct sockaddr*)NULL, NULL)) < 0) {
			die("can't accept");
			}
		
		/*searching for free thr element */

		for (i = 0;i < size;i++) if (clients[i].thr == 0) num = i;
		clients[num].fd = fd;
		
		/* creating thread */ 
        	i = pthread_create(&clients[num].thr,(pthread_attr_t *)NULL,thr_void,&num);
        	if (i) {
                	printf("Can`t create thread, returned value = %d \n" ,i);
                	exit(-1);
        	};
		// test
		//for (i = 0; i < size; i++) printf("!! %d %d \n",clients[i].thr,clients[i].fd);		
	 }

}













