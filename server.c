#include <sys/types.h>        
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>


void Close(int fd) {
    if (close(fd) == -1) {
        perror("close failed");
        exit(EXIT_FAILURE);
    }
}

int fd = -1;

void sighandler(int signal) {
    if (fd != -1 ) 
    {
        fsync(fd);
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        printf("Usage: ./server server_ip server_port\n");
        exit(EXIT_FAILURE);
    }

    const char* server_ip = argv[1];
    in_port_t server_port = (in_port_t)atoi(argv[2]);


    signal(SIGTERM, sighandler);
    signal(SIGHUP, sighandler);


    int listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd == -1) {
        perror("socket failed\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(server_port);
    if (!inet_aton(server_ip, &(address.sin_addr))) {
        perror("invalid server's ip\n");
        exit(EXIT_FAILURE);
    }

    if (bind(listen_sd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_sd, 10) == -1) {
        perror("listen failed\n");
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(listen_sd, F_GETFL);
    fcntl(listen_sd, flags | O_NONBLOCK);

    for(int n = 1; ;++n) {

        socklen_t addrsz = sizeof(address);

        int client_sd = accept(listen_sd, (struct sockaddr*)&address, &addrsz);
        while (client_sd == -1 && errno == EAGAIN)
        {
            sleep(1);
            client_sd = accept(listen_sd, (struct sockaddr*)&address, &addrsz);
        }
        
        if (client_sd == -1) {
            perror("accept failed\n");
            exit(EXIT_FAILURE);
        }


        if (fork() == 0) 
        {
            Close(listen_sd);


            char output_filename[17];

            sprintf(output_filename, "%d%s", n,".txt");

            fd = open(output_filename, O_CREAT | O_WRONLY, 0666);
            if (fd == -1) {
                perror("open failed\n");
                exit(EXIT_FAILURE);
            }


            char buf[8192]; 
            ssize_t sz = read(client_sd, buf, sizeof(buf));

            while(sz) 
            {
                write(fd, buf, sz);
                sz = read(client_sd, buf, sizeof(buf));
            }

            Close(fd);
            Close(client_sd);

            exit(EXIT_SUCCESS);
        }

        Close(client_sd);

        while (wait4(-1, NULL, WNOHANG, NULL) > 0) 
        {}
    }

    Close(listen_sd);
}
