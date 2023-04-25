#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

// Extract IP address from sockaddr struct
// void *get_in_addr(struct sockaddr *sa) {
//   return &(((struct sockaddr_in*)sa)->sin_addr);
// }

// Extract only the requested pathname from HTTP GET
char* parse_request(const char* request) {
  const char *path_start = strchr(request, ' ') + 1;
  const char *path_end = strchr(path_start, ' ');

  int path_size = (path_end - path_start);

  char *path = malloc(path_size * sizeof(char));

  strncpy(path, path_start, path_size);

  path[path_size] = 0;

  return path;
}

// Return a lower case version of given string
char* lowercase_string(const char* str) {
  char *result = malloc(sizeof(str));
  for (int i = 0; str[i]; i++) {
    result[i] = tolower(str[i]);
  }
  result[strlen(str)] = 0;
  return result;
}

// Check if requested pathname is in current directory
int check_request(int n, struct dirent **filelist, char* request) {
  char* request_lower = lowercase_string(request) + 1;
  for (int i = 0; i < n; i++) {
    char* name_lower = lowercase_string(filelist[i]->d_name);
    if (strcmp(request_lower, name_lower) == 0) {
      return i;
    }
  }
  return -1;
}

int main(int argc, char *argv[])
{
  //Create socket
  int my_sock = socket(PF_INET, SOCK_STREAM, 0);
  if (my_sock == -1) {
    perror("Socket initialization Failed");
    exit(-1);
  }

  //Option to prevent zombie sockets (address in use) after ^C
  int optval = 1;                                         
  setsockopt(my_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)); 

  //Specify port # and ip_address
  struct sockaddr_in my_port;
  my_port.sin_family = AF_INET;
  my_port.sin_port = htons(8080);                            
  my_port.sin_addr.s_addr = INADDR_ANY;                   
  memset(my_port.sin_zero, '\0', sizeof my_port.sin_zero);

  bind(my_sock, (struct sockaddr*)&my_port, sizeof(my_port));

  listen(my_sock, 1);
    

  //Get a list of all names in current directory
  struct dirent **filelist;
  int n = scandir(".", &filelist, NULL, alphasort);

  // for (int i = 0; i < n; i++) {
  //   printf("%s\n", lowercase_string(filelist[i]->d_name));
  //   printf("%s\n", filelist[i]->d_name);
  //   free(filelist[i]);
  // }
  // free(filelist);

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

    if (check_request(n, filelist, filename) != -1) {
      printf("Matched request for %s!\n\n", filename);
    } else {
      printf("404 requested page cannot be found\n\n");
    }

    
  }
}

// char *msg = "Fuck you bitch";
// int bytes_sent = send(new_sock, msg, strlen(msg), 0);
// printf("%d bytes sent!\n", bytes_sent);

// int filter(const struct dirent *entry) {
//   const char *filename = entry->d_name;
//   const char *pattern = ".(\\.html | \\.htm | \\.txt | \\.jpg | \\.jpeg | \\.png)";
//   regex_t regex;
//   if (regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE | REG_NOSUB) != 0) {
//     perror("Regex compilation failed");
//     exit(1);
//   }
//   if (regexec(&regex, filename, 0, NULL, 0) == 0) {
//     return 1;
//   }
//   return 0;
// }