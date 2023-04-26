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

// Extract only the requested pathname from HTTP GET
char* parse_request(const char* request) {
  const char *path_start = strchr(request, ' ') + 2;
  const char *path_end = strchr(path_start, ' ');

  int path_size = (path_end - path_start);

  char *path = malloc(path_size + 1);
  strncpy(path, path_start, path_size);
  path[path_size] = 0;
  return path;
}

// Return a lower case version of given string
char* lowercase_string(const char* str) {
  char *result = malloc(strlen(str) + 1);
  for (int i = 0; i < strlen(str); i++) {
    result[i] = tolower(str[i]);
  }
  result[strlen(str)] = 0;

  return result;
}

// Check if requested pathname is in current directory
int check_request(int n, struct dirent **filelist, char* request) {
  char* request_lower = lowercase_string(request);
  for (int i = 0; i < n; i++) {
    char* name_lower = lowercase_string(filelist[i]->d_name);
    if (strcmp(request_lower, name_lower) == 0) {
      free(name_lower);
      free(request_lower);
      return i;
    }
    free(name_lower);
  }
  free(request_lower);
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
    

  // Get a list of all names in current directory
  struct dirent **filelist;
  int n = scandir(".", &filelist, NULL, alphasort);

  // Pre-formatted response lines
  char* response_ok = "HTTP/1.0 200 OK\r\n";
  char* response_not_found = "HTTP/1.0 404 Not Found\r\n";

  char* type_html = "Content-Type: text/html\r\n";
  char* type_txt = "Contet-Type: text/plain\r\n";
  char* type_jpeg = "Content-Type: image/jpeg\r\n";
  char* type_png = "Content-Type: image/png\r\n";
  char* type_binary = "Content-Type: application/octet-stream\r\n";

  char* closing = "Connection: close\r\n";

  while(1) {

    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    addr_size = sizeof(client_addr);

    int new_sock = accept(my_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (new_sock == -1) {
      continue;
    }

    char buf[128];
    recv(new_sock, buf, sizeof(buf), 0);

    char *filename = parse_request(buf);
    printf("Received request for %s\n\n", filename);

    int index = check_request(n, filelist, filename);

    char *response_buffer;

    //404 Reply
    if (index == -1) {

      printf("Failed to match request for %s\n\n", filename);

      response_buffer = malloc(128);
      char *tracker;

      strncpy(response_buffer, response_not_found, strlen(response_not_found));
      tracker = response_buffer + strlen(response_not_found);

      strncpy(tracker, type_txt, strlen(type_txt));
      tracker = tracker + strlen(type_txt);

      char line3[27];
      sprintf(line3, "Content-Length: %d\r\n", 0);

      strncpy(tracker, line3, strlen(line3));
      tracker = tracker + strlen(line3);

      strncpy(tracker, closing, strlen(closing));
      tracker = tracker + strlen(closing);

      tracker[0] = 0;
      send(new_sock, response_buffer, strlen(response_buffer), 0);

    } 

    //200 Reply
    else {

      printf("Matched request for %s!\n\n", filename);

      FILE *f;
      f = fopen(filelist[index]->d_name, "rb");
      fseek(f, 0, SEEK_END);
      long fsize = ftell(f);
      if (fsize == -1) {
        perror("File size determination error!!!");
        exit(1);
      }
      rewind(f);

      response_buffer = malloc(fsize + 128);
      char *tracker;

      strncpy(response_buffer, response_ok, strlen(response_ok));
      tracker = response_buffer + strlen(response_ok);

      strncpy(tracker, type_jpeg, strlen(type_jpeg));
      tracker = tracker + strlen(type_jpeg);

      char line3[27];
      sprintf(line3, "Content-Length: %ld\r\n", fsize);

      strncpy(tracker, line3, strlen(line3));
      tracker = tracker + strlen(line3);

      strncpy(tracker, closing, strlen(closing));
      tracker = tracker + strlen(closing);

      fread(tracker, fsize, 1, f);
      fclose(f);
      tracker[fsize] = 0;

      write(new_sock, response_buffer, fsize);
      printf("%ld ", strlen(response_buffer));
      printf("%ld\n\n", fsize);
    }

    //Remember to set them free
    free(filename);
    free(response_buffer);
    close(new_sock);
  }
}

