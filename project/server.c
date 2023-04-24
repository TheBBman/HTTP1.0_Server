#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// void *get_in_addr(struct sockaddr *sa) {
//   return &(((struct sockaddr_in*)sa)->sin_addr);
// }

char* parse_request(const char* request) {
  const char *path_start = strchr(request, ' ') + 1;
  const char *path_end = strchr(path_start, ' ');

  int path_size = (path_end - path_start);

  char *path = malloc(path_size * sizeof(char));

  strncpy(path, path_start, path_size);

  path[path_size] = 0;

  return path;
}

int main(int argc, char *argv[])
{
  int my_sock = socket(PF_INET, SOCK_STREAM, 0);
  if (my_sock == -1) {
    perror("Socket initialization Failed");
    exit(-1);
  }

  struct sockaddr_in my_port;
  my_port.sin_family = AF_INET;
  my_port.sin_port = htons(8080);                             //Specified port #
  my_port.sin_addr.s_addr = INADDR_ANY;           //Testing on localhost
  memset(my_port.sin_zero, '\0', sizeof my_port.sin_zero);

  bind(my_sock, (struct sockaddr*)&my_port, sizeof(my_port));

  listen(my_sock, 1);
    
  while(1) {

    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    addr_size = sizeof(client_addr);

    int new_sock = accept(my_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (new_sock == -1) {
      continue;
    }

    // char address[INET_ADDRSTRLEN];
    // inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), address, sizeof(address));
    // printf("Got connection from %s\n", address);

    char buf[128];
    int bytes_received = recv(new_sock, buf, sizeof(buf), 0);
    // printf("%d bytes received!\n\n", bytes_received);
    // printf("%s\n\n", buf);

    char *filename = parse_request(buf);
    printf("Received request for %s\n\n", filename);

    close(new_sock);
  }
}

// char *msg = "Fuck you bitch";
// int bytes_sent = send(new_sock, msg, strlen(msg), 0);
// printf("%d bytes sent!\n", bytes_sent);
