
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Allows html files to be viewed within the HTML
char htmlheader[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html\r\n\r\n";

// Allows text files viewed within the HTML
char txtheader[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/plain\r\n\r\n";

// Allows images to be viewed within the HTML
char imageheader[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: image/jpeg\r\n\r\n";

// HTML page
char webpage[] =
"HTTP/1.1 200 Ok\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Test Title</title>\r\n"
"<style> body { background-image: url('https://wallpaperaccess.com/full/2909109.jpg');}</style>\r\n"

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
"<a href='http://localhost:2077/test.html'>Click here to open test.txt!</a><br>\r\n"
"<a href='test.html' download>Click here to download test.txt!</a></body><br>\r\n"
"</html>\r\n";

// !!! DON'T FORGET : modify localhost to 92.86.74.50

// Main function, variables in main() are not necessary - Course
int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr, client_addr;  // structure used by the server & server file descriptor
    socklen_t sin_len = sizeof(client_addr);
    int fd_server, fd_client;

    /* Storing the contents sent by the browser (a request) */

    int on1 = 1;  // reuse port on
    int on2 = 1;  // OOBINLINE on

    char buf[2048];                                    // server descriptor
    int fdimg;
    int fdtxt;
    int fdhtml;
    int sent;
    int len;

// Creation of the socket - Course
    if((fd_server = socket(AF_INET, SOCK_STREAM, 0)) == -1)
      {
        perror("[Error]: Socket");
        exit(1);
      }

    bzero(&server_addr,sizeof(server_addr));    // From Course || or use : setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on1, sizeof(int));

// Filling in the structure - Course
    server_addr.sin_family = AF_INET;                 // Establishing the family structure of sockets (internet)
    server_addr.sin_addr.s_addr = INADDR_ANY;         // Accepting any address
    server_addr.sin_port = htons(2077);               // Using the user port written

// Setting the option for reusing the port given
      setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, (void *) &on1, sizeof(int));

// Attaching the socket
    if(bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)  // sizeof(struct sockaddr) works as well - Course
      {
        perror("[Error]: bind");
        close(fd_server);
        exit(1);
      }

// Making the server "listen" for clients
    if(listen(fd_server, 5) == -1)
      {
        perror("[Error]: listen");
        close(fd_server);
        exit(1);
      }

  // Setting SO_OOBINLINE for sd (out-of-band data recived on the socket will be placed in the normal input queue / allows read or recv without needing the MSG_OOB flag)
        setsockopt(fd_server, SOL_SOCKET, SO_OOBINLINE, &on2, sizeof(int));


        printf("\n [Server]: The server has started... waiting for a client on the port 2077 (92.86.74.50:2077) \n\n");
        fflush(stdout);


    while(1)
      {

        len = sizeof(client_addr);

      //fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
      fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);

      if(fd_client < 0)
        {
          perror("[Error]: The connection to the client has failed...\n");
          continue;
        }

      printf("[Info]: A client has connected!\n");
      fflush(stdout);

      // We give the server time to send the data
      sleep(0);   // the thread waits for a signal or for the time given to pass (0 seconds)

// End of anything from the course


      if(!fork())
          {
            close(fd_server);
            memset(buf, 0, 2048);
            read(fd_client, buf, 2047);

            printf("%s", buf);

            if (!strncmp(buf, "GET /Files/test.jpg", 13))
                {
                  printf("[Info]: Request to send file... getting file... sending bytes...\n");
                  write(fd_client, imageheader, sizeof(imageheader) - 1);
                  fdimg = open("Files/test.jpg", O_RDONLY);
                  sent = sendfile(fd_client, fdimg, NULL, 1000000);
                  printf("[Info]: File sent... number of bytes sent : %d \n\n", sent);
                  close(fdimg);
                }
            else if (!strncmp(buf, "GET /test.txt", 13))
                {
                  printf("[Info]: Request to send file... getting file... sending bytes...\n");
                  write(fd_client, txtheader, sizeof(txtheader) - 1);
                  fdtxt = open("test.txt", O_RDONLY);
                  sent = sendfile(fd_client, fdtxt, NULL, 1000000);
                  printf("[Info]: File sent... number of bytes sent : %d \n\n", sent);
                  close(fdtxt);
                }
            else if (!strncmp(buf, "GET /test.html", 13))
                {
                  printf("[Info]: Request to send file... getting file... sending bytes...\n");
                  write(fd_client, htmlheader, sizeof(htmlheader) - 1);
                  fdhtml = open("test.html", O_RDONLY);
                  sent = sendfile(fd_client, fdhtml, NULL, 1000000);
                  printf("[Info]: File sent... number of bytes sent : %d \n\n", sent);
                  close(fdhtml);
                }
            else
                {
                  write(fd_client, webpage, sizeof(webpage) - 1);
                }

            close(fd_client);
            printf("[Info]: Closing the connection to the client...\n\n");
            exit(0);
         }
      close(fd_client);
     }
    return 0;
}

/*
  Proiect realizat de Stan Andrei - Vladut (2E3)

  Bibliografie :
  - Cursurile & Seminarele profesorilor Paun Andrei & Alboaie Lenuta
  - Tipurile de MIME : https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
  - Informatii despre implementarea html in c : https://www.i-programmer.info/programming/cc/9993-c-sockets-no-need-for-a-web-server.html




*/
