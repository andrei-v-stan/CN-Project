
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char htmlheader[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html\r\n\r\n";

char txtheader[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/plain\r\n\r\n";

char imageheader[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: image/jpeg\r\n\r\n";

char webpage[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Test Title</title>\r\n"
"<style> body { background-image: url('http://localhost:2077/Background.jpg'); background-repeat: repeat;} </style>\r\n"
"<style>"
"a:link {color: red;}"
"a:visited {color: red;}"
"a:hover {color: green;}"
"a:active {color: blue;}"
"</style>\r\n"
"<body>\r\n"
"<p><b><a href='http://localhost:2077/test.txt' target='_blank'>This is a link</a></b></p>"
"<a href='http://localhost:2077/test.txt'>Click here to open test.txt!</a><br>\r\n"
"<a href='test.txt' download>Click here to download test.txt!</a></body><br><br>\r\n"
"<a href='http://localhost:2077/test.html'>Click here to open test.html!</a><br>\r\n"
"<a href='test.html' download>Click here to download test.html!</a></body><br>\r\n"
"</html>\r\n";

int main(int argc, char *argv[])
{
    struct sockaddr_in server, from;
    socklen_t sin_len = sizeof(from);
    int sd, cd;

    int on1 = 1;
    int on2 = 1;

    char buf[2048];
    int fdimg;
    int fdtxt;
    int fdhtml;
    int sent;
    int len;

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
      {
        perror("[Error]: Socket");
        exit(1);
      }

    bzero(&server,sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(2077);

    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void *) &on1, sizeof(int));


    if(bind(sd, (struct sockaddr *) &server, sizeof(server)) == -1)  // sizeof(struct sockaddr) works as well - Course
      {
        perror("[Error]: bind");
        close(sd);
        exit(1);
      }

    if(listen(sd, 5) == -1)
      {
        perror("[Error]: listen");
        close(sd);
        exit(1);
      }

    setsockopt(sd, SOL_SOCKET, SO_OOBINLINE, &on2, sizeof(int));


    printf("\n [Server]: The server has started... waiting for a client on the port 2077 (92.86.74.50:2077) \n\n");
    fflush(stdout);


    while(1)
      {
        len = sizeof(from);

      cd = accept(sd, (struct sockaddr *) &from, &sin_len);

      if(cd < 0)
        {
          perror("[Error]: The connection to the client has failed...\n");
          continue;
        }

      printf("[Info]: A client has connected!\n");
      fflush(stdout);

      sleep(0);

      if(!fork())
          {
            close(sd);
            memset(buf, 0, 2048);
            read(cd, buf, 2047);

            printf("%s", buf);

            if (!strncmp(buf, "GET /Background.jpg", 13))
                {
                  printf("[Info]: Request to send file... getting file... sending bytes...\n");
                  write(cd, imageheader, sizeof(imageheader) - 1);
                  fdimg = open("Background.jpg", O_RDONLY);
                  sent = sendfile(cd, fdimg, NULL, 1000000);
                  printf("[Info]: File sent... number of bytes sent : %d \n\n", sent);
                  close(fdimg);
                }
            else if (!strncmp(buf, "GET /test.txt", 13))
                {
                  printf("[Info]: Request to send file... getting file... sending bytes...\n");
                  write(cd, txtheader, sizeof(txtheader) - 1);
                  fdtxt = open("test.txt", O_RDONLY);
                  sent = sendfile(cd, fdtxt, NULL, 1000000);
                  printf("[Info]: File sent... number of bytes sent : %d \n\n", sent);
                  close(fdtxt);
                }
            else if (!strncmp(buf, "GET /test.html", 13))
                {
                  printf("[Info]: Request to send file... getting file... sending bytes...\n");
                  write(cd, htmlheader, sizeof(htmlheader) - 1);
                  fdhtml = open("test.html", O_RDONLY);
                  sent = sendfile(cd, fdhtml, NULL, 1000000);
                  printf("[Info]: File sent... number of bytes sent : %d \n\n", sent);
                  close(fdhtml);
                }
            else
                {
                  write(cd, webpage, sizeof(webpage) - 1);
                }

            close(cd);
            printf("[Info]: Closing the connection to the client...\n\n");
            exit(0);
         }
      close(cd);
     }
    return 0;
}
