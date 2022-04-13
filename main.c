#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include "Base.h"
#include "TLSTransport.h"

int main() {
#ifdef SERVER_MODE

    pid_t child_pid;
    struct TLSConnectionState listeningSrvSock, receivingSrvSock;
    int resRecv;
    socklen_t cliAddrLen;
    char buf [1024];
    struct sockaddr_in addrSrv, addrCli;

    bzero(&addrSrv, sizeof(addrSrv));
    bzero(&addrCli, sizeof(addrCli));

    listeningSrvSock = tls_socket (AF_INET, SOCK_STREAM, 0);

    addrSrv.sin_family = AF_INET;
    addrSrv.sin_addr.s_addr = htonl (INADDR_ANY); // inet_addr (127.0.0.1);
    addrSrv.sin_port = htons(1234);

    if (tls_bind(&listeningSrvSock, (struct sockaddr*)&addrSrv, sizeof(addrSrv)) < 0 )
    {
        perror ("Binding problem");
        return -1;
    }
    
    tls_listen(&listeningSrvSock, 5);
    for(;;) {
        cliAddrLen = sizeof(addrCli);
        receivingSrvSock = tls_accept(&listeningSrvSock, (struct sockaddr *) &addrCli, &cliAddrLen);
        if (receivingSrvSock.socket_fd < 0) {
            perror("Problem of accepting");
            return -1;
        }
        fprintf(stdout, "Got a connection from ip: %s\n",
                inet_ntop(AF_INET, &(addrCli.sin_addr), buf, INET_ADDRSTRLEN));
        if ((child_pid = fork()) == 0) {
            tls_close(&listeningSrvSock);
            do {
                resRecv = tls_recv(&receivingSrvSock, buf, sizeof(buf), 0);
                if (resRecv)
                    fprintf(stdout, "%s", buf);
            } while (resRecv > 0);
            exit(0);
        }
        tls_close(&receivingSrvSock);
    }

    return 0;



#else

    struct TLSConnectionState socketFdCli;
    char ipSrv[256];
    char exitWord[6] = "FINISH";
    char textMsg[256];
    struct sockaddr_in addrSrv;

    printf("Input IP-address\n");
    scanf("%s", ipSrv);

    socketFdCli = tls_socket(AF_INET, SOCK_STREAM, 0);
    if (socketFdCli.socket_fd < 0 )
    {
        perror("Problem with creating socket");
        return -1;
    }

    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(1234);
    addrSrv.sin_addr.s_addr = inet_addr(ipSrv);

    if (tls_connect (&socketFdCli, (struct sockaddr*)&addrSrv, sizeof(addrSrv)) < 0)
    {
        perror("Connection problem");
        return -1;
    }

    printf("Connection established. Enter some text to send below:\n");

    while (fgets(textMsg, 256, stdin) != 0 && strcmp(textMsg, "FINISH\n") != 0)
    {
        if(tls_send(&socketFdCli, textMsg, strlen(textMsg) + 1, 5, 0) == 0){
            printf("Connection closed by server.\n");
            break;
        }
    }

    tls_close(&socketFdCli);
    return 0;


#endif


}
