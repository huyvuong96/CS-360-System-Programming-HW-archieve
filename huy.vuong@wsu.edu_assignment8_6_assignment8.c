#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

#define MY_PORT_NUMBER 49999

int main(int argc, char const *argv[])
{
	if(argc <= 1){
		printf("not enough argument\n");
		return 0;
	}

	else{
		if(strcmp(argv[1],"server") == 0){
			int listenfd;
			int connectfd;

			int connect_time = 0;
			//make socket
			listenfd = socket(AF_INET, SOCK_STREAM, 0);

			if (listenfd < 0){
		        perror("Error");
		        exit(1);
		    }

		    //bind socket
		    struct sockaddr_in servAddr;
			memset(&servAddr, 0, sizeof(servAddr));
			servAddr.sin_family = AF_INET;
			servAddr.sin_port = htons(MY_PORT_NUMBER);
			servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

			if (bind( listenfd,(struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
				perror("Error");
				exit(1);
			}

			//listen socket
			if (listen(listenfd, 1) < 0){
		        perror("Error");
		        exit(1);
		    }

		    //printf("Server start listening at port: %d\n", MY_PORT_NUMBER);

		    while(1){
		    	waitpid(-1, NULL, WNOHANG);

		    	//make connection
		    	int length = sizeof(struct sockaddr_in);
				struct sockaddr_in clientAddr;
				connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);

				if (connectfd < 0){
		            perror("Error");
		            exit(EXIT_FAILURE);
		        }

		        connect_time++;

		        //get host name
		        char hostName[NI_MAXHOST];
		        int hostEntry;
		        hostEntry = getnameinfo((struct sockaddr*)&clientAddr, sizeof(clientAddr), hostName, sizeof(hostName), NULL, 0, NI_NUMERICSERV);

		        if(hostEntry != 0){
		        	fprintf(stderr, "Error: %s\n", gai_strerror(hostEntry));
		        	exit(1);
		        }

		        printf("%s %d\n", hostName, connect_time);
		        fflush(stdout);

		        if(fork()){
		        	close(connectfd);
		        }
		        else{

		        	//get date and time
		        	time_t cur_date_time;
		        	time(&cur_date_time);
		        	char *date_time = ctime(&cur_date_time);

		        	if((write(connectfd, date_time, strlen(date_time))) < 0){
		        		perror("Error");
		        		exit(1);
		        	}

		        	exit(1);
		        }
		    }

		    return 0;
		}

		else if(strcmp(argv[1],"client") == 0){

			int socketfd;
			char response[18];

			if(argc != 3){
				perror("Error");
				exit(1);
			}

			else{

				//set up the address of the server
				struct addrinfo hints, *actualdata;
				memset(&hints, 0, sizeof(hints));
				int err;

				hints.ai_socktype = SOCK_STREAM;
				hints.ai_family = AF_INET;

				err = getaddrinfo(argv[2], "49999", &hints, &actualdata);

				if(err != 0){
					fprintf(stderr,"Error: %s\n", gai_strerror(err));
					exit(1);
				}

				socketfd = socket(actualdata -> ai_family, actualdata -> ai_socktype, 0);

				if(connect(socketfd, actualdata -> ai_addr, actualdata -> ai_addrlen) < 0){
					perror("Error");
					exit(1);
				}

				read(socketfd, response, 18);
				printf("%s\n", response);
				close(socketfd);
				return 0;


			}

		}

		else{
			printf("invalid argument\n");
			exit(1);
		}
	}
	
}