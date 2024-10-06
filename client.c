#include <sys/types.h>        
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage: ./client path_to_file server_ip server_port\n");
        exit(EXIT_FAILURE);
    }

    const char* path_to_file = argv[1];
    const char* server_ip = argv[2];
    in_port_t server_port = (in_port_t)atoi(argv[3]);


    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("socket failed\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = 0;
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_aton(server_ip, &(server_address.sin_addr)) == 0) {
        perror("invalid server's ip\n");
        exit(EXIT_FAILURE);
    }

    if (bind(sd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("bind failed\n");
        exit(EXIT_FAILURE);
    }

    if (connect(sd, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        perror("connect failed\n");
        exit(EXIT_FAILURE);
    };

    int fd = open(path_to_file, O_RDONLY);
    if (fd == -1) {
        perror("open failed\n");
        exit(EXIT_FAILURE);
    }

    char buf[8192];

    ssize_t sz = read(fd, buf, sizeof(buf));

    while (sz > 0) 
    {
        if (write(sd, buf, sz) == -1) {
            perror("write failed\n");
            exit(EXIT_FAILURE);
        }
        sz = read(fd, buf, sizeof(buf));
    }

    if (close(fd) == -1) {
        perror("file close failed\n");
        exit(EXIT_FAILURE);
    }
    if (close(sd) == -1) {
        perror("socket close failed\n");
    }
}
