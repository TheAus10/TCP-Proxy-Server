// NAME: AUSTIN REEVES
// EUID: AWR0047
// DATE: 2/25/2021

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


int main(int argc, char *argv[])
{
	// variables for socket connections
	int sockfd = 0, n = 0, portno;
	struct hostent *server;
	struct sockaddr_in svr_addr;

	char svr_reply[40960];	// string for server messages
	int maxfd;      				// descriptors up to maxfd-1 polled
  int nready;     				// # descriptors ready
  int nread;      				// returned from read
  fd_set fds;     				// set of file descriptors to poll

	// usage check
	if (argc != 2)
  {
	 		fprintf(stderr, "usage: %s [svr_port]\n", argv[0]);
      exit(EXIT_FAILURE);
  }

	// setting up socket
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
			printf("socket error\n");
			exit(EXIT_FAILURE);
	}

	// connecting to cse02.cse.unt.edu
	server = gethostbyname("cse02");
  if (server == NULL)
  {
			fprintf(stderr, "error: host 'cse02' does not exist\n");
	    exit(EXIT_FAILURE);
	}

	// setting address
	portno = atoi(argv[1]);
  bzero((char *) &svr_addr, sizeof(svr_addr));
  svr_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&svr_addr.sin_addr.s_addr, server->h_length);
  svr_addr.sin_port = htons(portno);

	// connecting to server
	if (connect(sockfd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr)) == -1)
	{
			printf("connect error\n");
			exit(EXIT_FAILURE);
	}

	printf("\n\n*** Connected ***\n\n");
	printf("\tWelcome to the Proxy Server!\n");
 	printf("\tEnter a url and the server will\n");
	printf("\treturn the received page.\n");
	printf("\tType \"quit\" to exit the program\n");

	maxfd = sockfd + 1;

	// loop forever until client says 'quit'
	while (1)
  {

    	fflush(stdout);

      // set up polling using select
      FD_ZERO(&fds);
      FD_SET(sockfd, &fds);
      FD_SET(0, &fds);

			// waiting for an input
      nready = select(maxfd, &fds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0);

      // checks for input from server
      if (FD_ISSET(sockfd, &fds))
      {
					bzero(svr_reply, sizeof(svr_reply)); // clear out svr_reply message

          if ((nread = recv(sockfd, svr_reply, sizeof(svr_reply), 0)) == -1)
          {
							perror("recv error");
              exit(EXIT_FAILURE);
          }
					if(nread < 1)
					{
							close(sockfd);
							exit(0);
					}

      		printf("%s", svr_reply);
			}

			// checks for input from stdin
			if (FD_ISSET(0, &fds))
      {
					memset(&svr_reply[0], 0, sizeof(svr_reply));
					nread = read(0, svr_reply, sizeof(svr_reply));

          if (send(sockfd, svr_reply, strlen(svr_reply), 0) == -1)
          {
							perror("send error");
              exit(EXIT_FAILURE);
          }
      }
	}

  close(sockfd);
	return 0;
}
