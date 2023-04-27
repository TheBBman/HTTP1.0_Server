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

// Deal with % and spaces, this took way longer than I expected
char* process_string(char* string, int mode) {

    char *result;
    char *insert;
    char *temp;
    char *replace;
    char *with;
    char *string2 = string;
    int count = 0;

    // 0 means replace %, 1 means replace empty space
    switch(mode) {
        case 0:
            replace = "%25";
            with = "%";
            break;
        case 1:
            replace = "%20";
            with = " ";
            break;
        default:
    }

    // Count substrings # to be replaced
    insert = string;
    for (count; temp = strstr(insert, replace); count++) {
        insert = temp + 3;
    }

    // If nothing needs to be replaced, return original string
    if (!count) {
        return string;
    }

    // Allocate new string buffer
    temp = result = malloc(strlen(string) - 2*count + 1);

    // While still substrings to be replaced
    while (count--) {

        // Find insert point
        insert = strstr(string, replace);
        // Find distance from start or previous replacement
        int jump = insert - string;
        // Copy over non-replacement part
        temp = strncpy(temp, string, jump) + jump;
        // Replace 
        temp = strcpy(temp, with) + 1;
        // Update replacement point
        string += jump + 3;
    }
    strcpy(temp, string);
    free(string2);
    return result;
}

// Extract only the requested pathname from HTTP GET
char* parse_request(const char* request) {
  const char *path_start = strchr(request, ' ') + 2;
  const char *path_end = strchr(path_start, ' ');

  int path_size = (path_end - path_start);

  char *path = malloc(path_size + 1);
  strncpy(path, path_start, path_size);
  path[path_size] = 0;
  path = process_string(path, 0);
  path = process_string(path, 1);
  return path;
}

// Return int identifier for switch case based on fle extension
int get_extension(const char* path) {
  const char *extension = strchr(path, '.');
  if (extension == NULL) {
    return 0;
  }

  if ( (strcmp(extension, ".html") == 0) || (strcmp(extension, ".htm") == 0) ) {
    return 1;
  } else if ( (strcmp(extension, ".txt") == 0) ) {
    return 2;
  } else if ( (strcmp(extension, ".jpg") == 0) || (strcmp(extension, ".jpeg") == 0) ) {
    return 3;
  } else if ( (strcmp(extension, ".png") == 0) ) {
    return 4;
  } else {
    return 0;
  }
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
  for (int i = 0; i < n; i++) {
    char* name_lower = lowercase_string(filelist[i]->d_name);
    if (strcmp(request, name_lower) == 0) {
      free(name_lower);
      return i;
    }
    free(name_lower);
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
    

  // Get a list of all names in current directory
  struct dirent **filelist;
  int n = scandir(".", &filelist, NULL, alphasort);

  // Pre-formatted response lines
  char* response_ok = "HTTP/1.0 200 OK\r\n";
  char* response_not_found = "HTTP/1.0 404 Not Found\r\n";

  char* type_html = "Content-Type: text/html\r\n";
  char* type_txt = "Content-Type: text/plain\r\n";
  char* type_jpeg = "Content-Type: image/jpeg\r\n";
  char* type_png = "Content-Type: image/png\r\n";
  char* type_binary = "Content-Type: application/octet-stream\r\n";

  char* closing = "Connection: close\r\n\r\n";

  printf("Setup complete!\n\n");
  while(1) {

    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    addr_size = sizeof(client_addr);

    int new_sock = accept(my_sock, (struct sockaddr *)&client_addr, &addr_size);
    if (new_sock == -1) {
      continue;
    }

    printf("Listening!\n\n");

    char buf[128];
    recv(new_sock, buf, sizeof(buf), 0);

    char *filename = lowercase_string(parse_request(buf));
    printf("Received request for %s\n\n", filename);

    int index = check_request(n, filelist, filename);

    char *file_buffer;

    //404 Reply
    if (index == -1) {

      printf("Failed to match request for %s\n\n", filename);

      write(new_sock, response_not_found, strlen(response_not_found));

      write(new_sock, type_html, strlen(type_html));

      char line3[27];
      sprintf(line3, "Content-Length: %ld\r\n", 0);

      write(new_sock, line3, strlen(line3));

      write(new_sock, closing, strlen(closing));

      //index = check_request(n, filelist, "404notfound.html");
      free(filename);
    } 
    //200 Reply
    else {

      printf("Matched request for %s!\n\n", filename);

      write(new_sock, response_ok, strlen(response_ok));

      char* type;
      switch(get_extension(filename)) {
        case 0: 
          type = type_binary;
          break;
        case 1:
          type = type_html;
          break;
        case 2:
          type = type_txt;
          break;
        case 3:
          type = type_jpeg;
          break;
        case 4:
          type = type_png;
          break;
        default: 
          type = type_binary;
          break;
      }
      write(new_sock, type, strlen(type));

      FILE *f;
      f = fopen(filelist[index]->d_name, "rb");
      fseek(f, 0, SEEK_END);
      long fsize = ftell(f);
      rewind(f);

      char line3[27];
      sprintf(line3, "Content-Length: %ld\r\n", fsize);

      write(new_sock, line3, strlen(line3));

      write(new_sock, closing, strlen(closing));

      file_buffer = malloc(fsize);

      fread(file_buffer, fsize, 1, f);
      fclose(f);

      long total_transmitted = 0;
      while(total_transmitted < fsize) {
        total_transmitted += write((new_sock + total_transmitted), file_buffer, fsize);
      }

      //Remember to set them free
      free(filename);
      free(file_buffer);
    }

    
  } 


}

