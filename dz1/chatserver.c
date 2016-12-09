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

FILE *wfd;
const char *fname = "chat_history.txt";
 
pthread_mutex_t lock;

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

struct USERS {
	char name[20];
	int key[2];
	int hash;
} users[size]; // base of users with key and hash


int num; // number of client

int my_hash(char *key, char *pass){
	int hash = key[0]*100000 + key[1]*10000 + pass[0]*1000 + pass[1]*100 + pass[2]*10 + pass[3];
	return hash;
}



/* threads receive messages and send them to everybody except sender */ 

void* thr_void(void* arg)
	{
		int len,i,fd;
		char name[20];
		char password[4];
		int k = *(int *)arg;
		struct buff recv_buf,send_buf; 
                fd = clients[k].fd;

		/* receive a name*/

		len = recv(fd,name,20,0);
		if (len < 0) die("receiving a name failed");
		printf("%s, thank you for visiting us! \n",name);
		int iter = 0;
		int found = 0;
		int cur = 0;
		while(iter < size){
			if(users[iter].name[0] == 0){
				cur = iter;
				iter = size;
				printf("name 0\n");	
			}
			if(strncmp(users[iter].name,name,sizeof(name)) == 0){
				printf("found name\n");
				if(len = recv(fd, password,4,0) < 0) die("cant reciew paw");
				//char plsenter[] = {'p','l','s',' ','e','n','t','r',' ','p','s','w',':'};
				//if(len = send(fd, plsenter,13, 0) < 0) die("can't send");
				int cur_hash = my_hash(users[iter].key, password);				
				if(users[iter].hash == cur_hash){
					iter = size;	
					found = 1;
				}else{
					iter = size;
					found = 2;
					printf("wrong password\n");
				}
			}

			++iter;
		}
		
		if(found == 0){
			users[cur+1].name[0] = 0;
			strcpy(users[cur].name, name);
			users[cur].key[0] = rand() %10;
			users[cur].key[1] = rand() %10;
			//char plsenter[] = {'p','l','s',' ','e','n','t','r',' ','p','s','w',':'};
                        //if(len = send(fd, plsenter,13, 0) < 0) die("can't send");
			if(len = recv(fd, password, 4, 0) < 0) {die("cant reciev password");}	
			printf("created a user\n");
			users[cur].hash  = my_hash(users[iter].key, password);				}


		/* main cycle */
		if(found != 2){
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
				if(pthread_mutex_lock(&lock)) die("can't lock");
					if((wfd = fopen(fname, "a")) == NULL) die("can't open file");
					size_t count;
					if((count = fwrite(send_buf.name, strlen(send_buf.name), 1, wfd)) != 1) die("error writing file");
					if((count = fwrite(send_buf.msg, strlen(send_buf.msg), 1, wfd)) != 1) die("error writing file");
					fclose(wfd);
				if(pthread_mutex_unlock(&lock)) die("can't unlock");

				printf("sent to all by %s \n",name);
			}	
		}
		char mserver[6] = {'s','e','r','v','e','r'};
		char msger[14] = {'w','r','o','n','g'};
		strcpy(send_buf.name,mserver);
		strcpy(send_buf.msg,msger);
		if(len = send(fd, &send_buf, 1020, 0) < 0) die("cant send");
		printf("%s left us \n",name);
		close(fd);
		clients[k].thr = 0; // closing descriptor of client and zero corresponding thr
	};
	
int main() {

	pthread_mutex_init(&lock, NULL);

	users[0].name[0] = 0;
	
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





