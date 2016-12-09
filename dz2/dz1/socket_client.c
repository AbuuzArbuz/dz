#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <pthread.h>

#define length 50

void die(char *msg) {
	printf("error: %s, errno: %d\n", msg, errno);
	// grep 'define' /usr/include/asm-generic/errno.h  | grep <errno>
	exit(1);
}

time_t ticks;

struct buff {
		char name[20];
		char msg[1000]; 
	}  send_buff,recv_buff;

void* my_void(void* arg)
	{
		int sfd = *(int *)arg;
		char str[1050];
		while(1) {
			if (recv(sfd,&recv_buff, 1020, 0) < 0) {
 		       		die("can't recv");
 			}
		/* form string */

		ticks = time(NULL);
		strcpy(str,recv_buff.name);
		strcat(str,": ");
		puts(str);
		int i = 0,k,len,fl = 0;
		char * msg = malloc(1000*sizeof(char));
		strcpy(msg,recv_buff.msg);
		while(strlen(msg) >= length) {
			if (msg[0] == ' ') msg++; // delete a space in the beginning
			while(i <= length) {
				if (msg[i] == ' ') {
						  // if msg[i] = 'space' then check if we dont reach the end of the string
					strncpy(str,msg,i); // copy first i symbols to str (before space)	
					str[i] = 0;
					len = i;
					fl = 1;
				}
				if ((i == length)&&(fl == 0)) {
					strncpy(str,msg,i);
					str[i] = 0;
					len = i;
				}
				i++;	
			}
			for(k = strlen(msg); k <= length; k++)
                        	strcat(str," "); // add spaces until the end of the string	
			puts(str); // str to screen
			msg += len; // shift of msg	
			i = 0;
			len = 0;
			fl = 0;
		} 
		if (msg[0] == ' ') msg++;
		strcpy(str,msg);
		str[strlen(msg)] = 0;
		for(k = strlen(msg); k <= length; k++) 
			strcat(str," "); // add spaces until the end of the string

		strcat(str,ctime(&ticks));
		str[strlen(str)-1] = 0;
		puts(str); // str to screen
		printf("\n");
		}
	}

int main(int argc, char *argv[]) {
	int sockfd = 0, recv_len = 0;
	struct sockaddr_in serv_addr; 

	if(argc != 3) {
		printf("\n Usage: %s <ip of server> <nickname> \n",argv[0]);
		return 1;
	} 

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		die("can't create socket");
	} 

	memset(&serv_addr, '0', sizeof(serv_addr)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000); 

	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
		die("inet_pton error occured");
	}

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		die("Connect Failed");
	}
	
	/* say my name */ 

	if (send(sockfd, argv[2], 20, 0) < 0) {
			die("can't send");
		}
	
	/* creating thread */

	int i;
	pthread_t my_thr;
      	i = pthread_create(&my_thr,(pthread_attr_t *)NULL,my_void,&sockfd);

	/* cycle of sending */

	while(1) {
		fgets(send_buff.msg,1000,stdin);
		printf("\n");
		send_buff.msg[strlen(send_buff.msg)-1] = 0;
		if (send(sockfd, send_buff.msg, 1000, 0) < 0) {
			die("can't send");
			}
	}

	close(sockfd);

	return 0;
}




