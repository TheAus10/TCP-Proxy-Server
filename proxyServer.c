// NAME: AUSTIN REEVES
// EUID: AWR0047
// DATE: 2/25/2021

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <time.h>


int main(int argc, char *argv[])
{
    int listenfd = 0, cli_sockfd = 0, web_sockfd = 0;                 // all file descriptors
    int cli_size, portno, nread;                                      // size of client address, port number used by client, and recieved message int
    int len = sizeof(struct sockaddr);                                // length of address

	struct sockaddr_in proxy_svr_addr, cli_addr, web_svr_addr;        // all socket addresses
    struct hostent *webServerName;                                    // the web server named by the client

    char cliMsgSend[40960];                                           // message sent to client
	char cliMsgRecv[40960];                                           // message recieved from client
    char svrMsgSend[40960];                                           // message sent to web server
    char svrMsgRecv[40960];                                           // message recieved from web server
    char ip_addr[256];                                                // holds the ip address of the named server

    char cache[6][256] = {0};                                         // array for the cache system. holds 6 websites each with a name of 256 character length
    int  foundInCache = 0;                                            // boolean that is true when a website is found in the cache
    FILE *fp;                                                         // file pointer for the file I/O
    char timeStamp[128];                                              // holds the timestamp at cache[i]
    char hostName[128];                                               // holds the hostname at cache[i]
    char temp[256];                                                   // temp string for cache manipulation

    time_t now;                                                       // holds the current time when user gets file
	struct tm nowLocal;                                               // formats time to be locally correct

	// checking for valid number of arguments
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <svr_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // opening up cache file and storing contents into cache array
    fp = fopen("list.txt", "r");
    int n = 0;
    while(fgets(temp, sizeof(temp), fp) != NULL)
    {
        // removing the added newline
        strtok(temp, "\n");
        strcpy(cache[n], temp);
        n++;
    }
    fclose(fp);

    // printing out cache
    printf("----------------------------\n*** Contents of Cache:\n");
    for(int i = 0; i < 6; i++)
    {
        printf("[%d] %s\n", i, cache[i]);
    }
    printf("----------------------------\n\n");

    // telling user that the system is waiting
    printf("Waiting for connection...\n");

    // listening for the client socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	printf("socket error\n");
    	exit(EXIT_FAILURE);
    }

    // allocating buffer memory
    memset(&proxy_svr_addr, '0', sizeof(proxy_svr_addr));
    memset(&web_svr_addr, '0', sizeof(web_svr_addr));
    memset(cliMsgRecv, '0', sizeof(cliMsgRecv));
    memset(cliMsgSend, '0', sizeof(cliMsgSend));
    memset(svrMsgRecv, '0', sizeof(svrMsgRecv));
    memset(svrMsgSend, '0', sizeof(svrMsgSend));

    // setting address of proxy server
    proxy_svr_addr.sin_family = AF_INET;
    proxy_svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    portno = atoi(argv[1]);
    proxy_svr_addr.sin_port = htons(portno);

    // binding the client and proxy server sockets
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if(bind(listenfd, (struct sockaddr *)&proxy_svr_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("bind error\n");
    	exit(EXIT_FAILURE);
    }

    // listening for cnnection by client
    if (listen(listenfd, 10) == -1)
    {
    	printf("listen error\n");
    	exit(EXIT_FAILURE);
    }

    // accepting the connection
    cli_size = sizeof(cli_addr);
    if ((cli_sockfd = accept(listenfd, (struct sockaddr *)&cli_addr, &cli_size)) == -1)
    {
        printf("accept error\n");
        exit(EXIT_FAILURE);
    }

    printf("*** Client Connected ***\n\n");

    // looping forever until user enters quit
    while (1)
    {

        // resetting boolean each loop
        foundInCache = 0;

    	// clearing buffers
        bzero(cliMsgSend, sizeof(cliMsgSend));
    	bzero(cliMsgRecv, sizeof(cliMsgRecv));
        bzero(svrMsgSend, sizeof(svrMsgSend));
        bzero(svrMsgRecv, sizeof(svrMsgRecv));
        bzero(timeStamp, sizeof(timeStamp));
        bzero(hostName, sizeof(hostName));
        bzero(temp, sizeof(temp));

    	// waiting for an input string
    	strcpy(cliMsgSend, "\nurl: ");
        write(cli_sockfd, cliMsgSend, strlen(cliMsgSend));
        puts("Waiting for url...\n");
    	nread = recv(cli_sockfd, cliMsgRecv, sizeof(cliMsgRecv), 0);
        puts(cliMsgRecv);
        fflush(stdout);

        // checking for quit message from user
    	if(strcmp(cliMsgRecv, "quit\n") == 0)
    	{
    	    strcpy(cliMsgRecv, "Goodbye!\n");
            write(cli_sockfd, cliMsgRecv, strlen(cliMsgRecv));
    		close(cli_sockfd);
            return 0;
    	}

        // removing newline charcter from client reieved string
        strtok(cliMsgRecv, "\n");

        // setting up socket to port number 80 for internet
        web_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&web_svr_addr,sizeof(web_svr_addr));
        web_svr_addr.sin_family = AF_INET;
        web_svr_addr.sin_port = htons(80);

        // connecting to the server entered by the client
        webServerName = gethostbyname(cliMsgRecv);
        if (webServerName == NULL)
        {
    		fprintf(stderr, "error: host '%s' does not exist\n", cliMsgRecv);
            sprintf(cliMsgSend, "error: host '%s' does not exist\n", cliMsgRecv);
            write(cli_sockfd, cliMsgSend, strlen(cliMsgSend));
    	    continue;
    	}

        // getting the address from the named server
        for(int i = 0; webServerName->h_addr_list[i]; i++)
        {
            web_svr_addr.sin_addr = *((struct in_addr*) webServerName->h_addr_list[i]);
            inet_ntop(AF_INET, &web_svr_addr.sin_addr, ip_addr, sizeof(ip_addr));
            printf("%s\n", ip_addr);
        }

        // connecting to the web server
        connect(web_sockfd, (struct sockaddr*)&web_svr_addr, sizeof(web_svr_addr));

        // checking if the given website is already in the cache
        for(int i = 0; i < 6; i++)
        {
            // checks if the spot in the array is empty to skip over it
            if(strcmp(cache[i], "\0") != 0)
            {
                // parsing cache[i] into its hostname and timestamp
                strcpy(temp, cache[i]);
                strcpy(hostName, strtok(temp, " "));
                strcpy(timeStamp, strtok(NULL, " "));

                // checks if the given url is in the current array location
                if(strcmp(hostName, cliMsgRecv) == 0)
                {
                    // site is already in cache, find file and return contents to user
                    foundInCache = 1;
                    puts("*** Client URL found in cache ***\n");

                    fp = fopen(timeStamp, "r");
                    while(fgets(cliMsgSend, sizeof(cliMsgSend), fp) != NULL)
                    {
                        // sending back the file contents
                        write(cli_sockfd, cliMsgSend, strlen(cliMsgSend));
                    }
                    fclose(fp);
                    break;
                }
            }
        }

        // making sure bufers are clear
        bzero(timeStamp, sizeof(timeStamp));
        bzero(hostName, sizeof(hostName));
        bzero(temp, sizeof(temp));

        // send request to server only if the given url wasn't found in the cache
        if(!foundInCache)
        {
            puts("*** Retrieving URL from web server ***\n");

            // formatting the GET statment
            sprintf(svrMsgSend, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", cliMsgRecv);
            puts(svrMsgSend);
            fflush(stdout);

            // sending GET statent to server
            if (send(web_sockfd, svrMsgSend, strlen(svrMsgSend), 0) == -1)
            {
                perror("send error");
                exit(EXIT_FAILURE);
            }

            // giving the server a second to retrieve the information
            sleep(1);

            // receiving the results from the server
            if ((nread = recv(web_sockfd, svrMsgRecv, sizeof(svrMsgRecv), 0)) == -1)
            {
                perror("recv error");
                exit(EXIT_FAILURE);
            }

            // generate timestamp
            now = time(NULL);
            nowLocal = *localtime(&now);
            sprintf(timeStamp, "%d%d%d%d%d%d", (nowLocal.tm_year + 1900), (nowLocal.tm_mon + 1), nowLocal.tm_mday, nowLocal.tm_hour, nowLocal.tm_min, nowLocal.tm_sec);

            // checking if server response was '200'
            memcpy(temp, &svrMsgRecv[9], 3);
            if(strcmp(temp, "200") == 0)
            {
                // checks for an empty spot in array
                for(int i = 0; i < 6; i++)
                {
                    if(strcmp(cache[i], "\0") == 0)
                    {
                        // this spot in the array is empty so add the webiste to it
                        foundInCache = 1;
                        sprintf(cache[i], "%s %s", cliMsgRecv, timeStamp);

                        // write results to file with name 'timeStamp'
                        fp = fopen(timeStamp, "w");
                        fputs(svrMsgRecv, fp);
                        fclose(fp);

                        break;
                    }
                }

                // happens only when the cache is full
                if(!foundInCache)
                {
                    // removing the file corresponding to cache[0]
                    strcpy(temp, cache[0]);
                    strcpy(hostName, strtok(temp, " "));
                    strcpy(timeStamp, strtok(NULL, " "));
                    remove(timeStamp);

                    // shifting everything up 1 array spot
                    for(int j = 0; j < 6; j++)
                    {
                        if(j < 5)
                        {
                            strcpy(cache[j], cache[j+1]);
                        }
                        else
                        {
                            // on the last loop, saving url to last spot in cache
                            sprintf(cache[5], "%s %s", cliMsgRecv, timeStamp);
                        }
                    }

                    // write results to file with name 'timeStamp'
                    fp = fopen(timeStamp, "w");
                    fputs(svrMsgRecv, fp);
                    fclose(fp);
                }
            }

            // sending back the server results
            sprintf(cliMsgSend, "\nResult:\n%s\n", svrMsgRecv);
            write(cli_sockfd, cliMsgSend, strlen(cliMsgSend));
        }

        // printing contents of the cache and writing it to file
        fp = fopen("list.txt", "w");
        printf("--------------------------------------\n*** Contents of Cache:\n");
        for(int i = 0; i < 6; i++)
        {
            printf("[%d] %s\n", i, cache[i]);
            if(strcmp(cache[i], "\0") != 0)
            {
                fputs(cache[i], fp);
                fputs("\n", fp);
            }
        }
        printf("--------------------------------------\n\n");
        fclose(fp);
    }
    return 0;
}
