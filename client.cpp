// client side c++ program to using Socket to connect to Google public DNS
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include "encode.h"
#define PORT 53
#define MAXLINE 1024

int main(int argc, char const* argv[]) {
  int status, valread, client_fd;
  const char *dns_ip = "8.8.8.8";
  // char buffer[MAXLINE];
  struct sockaddr_in serv_addr;
  if ((client_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("\n Socket creation error \n");
    exit(EXIT_FAILURE);
  }

  // set server address to be all 0s
  memset(&serv_addr, 0, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  if (inet_pton(AF_INET, dns_ip, &serv_addr.sin_addr) <= 0) {
    perror("\n Invalid address/ Address not supported \n");
    exit(EXIT_FAILURE);
  }
  // serv_addr.sin_addr.s_addr = htonl (0xd043dede);
  // htons() function converts the unsigned short integer hostshort
  // from host byte order to network byte order
  serv_addr.sin_port = htons(PORT);

  printf("TEST\n");
  uint8_t* packet;
  const char* hostname = "www.example.com";
  build_packet(packet, hostname);
  sendto(client_fd, packet, sizeof(packet), 0,(const struct sockaddr *) &serv_addr, sizeof(serv_addr));
  printf("SENT\n");
  socklen_t length = 0;
  uint8_t response[512];
  memset (&response, 0, 512);

  /* Receive the response into a local buffer */
  ssize_t bytes = recvfrom (client_fd, response, 512, MSG_WAITALL, (struct sockaddr *) &serv_addr, &length);
  printf("RECVD\n");
  // int n;
  // socklen_t len;
  // n = recvfrom(client_fd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &serv_addr, &len);
  // // printf("can_receive\n");
  // buffer[n] = '\0';
  // printf("Server : %s", buffer);

  close(client_fd);
}
