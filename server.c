#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

/* the logic of my program/server
   1: make sure the user started me with the right parameters
   2: create a socket (STREAM/connection oriented)
   3: Get the size of the filename the client sent
   4) Get the filename the client sent
   5) Print filename
   6) close socket and wait for another client 
*/


int main(int argc, char *argv[])
{
  int sd; /* socket descriptor */
  int flag; 
  int connected_sd; /* socket descriptor */
  int rc; /* return code from recvfrom */
  struct sockaddr_in server_address;
  struct sockaddr_in from_address;
  char buffer[100];
  socklen_t fromLength;
  int portNumber;

  if (argc < 2){
    printf ("Usage is: server <portNumber>\n");
    exit (1);
  }

  portNumber = atoi(argv[1]);
  sd = socket (AF_INET, SOCK_STREAM, 0); // create a socket. Check the rc in case you are out
  if (sd <0){
    printf ("error on socket creation\n");
    exit (1);
  }

  // The next stuff is all about creating and assigning my address for this socket

  fromLength = sizeof(struct sockaddr_in);
  server_address.sin_family = AF_INET; // This means i am using IPv4 addresses
  server_address.sin_port = htons(portNumber); // Note the htons(). that converts it to network order, 16 bit
  server_address.sin_addr.s_addr = INADDR_ANY; // This means any adapter on my machine

  // Since i am a server, i need to bind to the address
  rc = bind (sd, (struct sockaddr *)&server_address, sizeof(server_address));

  // Always check the RC, since someone else may be using my port number. 
  if (rc < 0){
    perror ("bind");
    exit(1);
  }

  // since i am a server i tell the system how many incoming 'connect' requests can be queued 
  listen (sd, 5);

  // in my server i will run forever! just like an http server might do
  while (1){
    flag = 0;

    // since i am a server, i need to wait for an incoming 'connect' from a client. Once that connect is sent,
    // the 'accept' will create a new socket, mapping my IP/Port and the client's IP/port
    connected_sd = accept (sd, (struct sockaddr *) &from_address, &fromLength);

    // always good in C to wipe your buffer clean before using
    memset(buffer, 0, 100);

    int sizeOfFilename;
    flag = 0;

    // in this example, the other side will send a size of a filename, then send the filename.
    // Note: to receive an integer, i can pass in the address of the integer i want to receive,
    // and the size of the int
    rc = read(connected_sd, &sizeOfFilename, sizeof(int));

    // rc will tell me how many bytes were read to fill in the sizeOfFilename variable. Most likely
    // 4 bytes in must *nix implementations
    // if the number of bytes is less than or = to 0, then i can assume the other side died
    if (rc <=0){
      printf ("the other side closed the socket\nI will clean up and wait for another connection\n");
      break;
    }

    // I like to print what i got just to make sure i did it right!
    printf ("read %d bytes to get the filesize\n", rc);
    printf("the size of the filename before converting is %d bytes\n",
	   sizeOfFilename);
      
    // In the internet, when sending  numbers, we convert them to network order (due to endianess)
    sizeOfFilename = ntohl(sizeOfFilename); // convert from network to host order
    printf("the size of the filename after converting is %d bytes\n",
	   sizeOfFilename);

    // We will learn this in class, but on connection oriented (STREAM) sockets, you are not guaranteed
    // than all the bytes sent by the client will show up at once! weird :-)

    // since the client told me they are going to send 'sizeOfFilename' bytes, i will keep
    // reading until i get them all

    int totalBytes = 0;
    char *ptr = buffer; // C loves pointers, makes it easier for me to read into the buffer this way

    while (totalBytes < sizeOfFilename){ // keep reading until i get exactly the number of bytes i am expecting
      rc = read(connected_sd, ptr, sizeOfFilename-totalBytes);
      // remember that rc contains the number of bytes i received. 
      if (rc <=0){
	printf ("this is awkward.. the other side quit while sending \n");
	flag = 1;
	break;
      }
      // keeping track of number of bytes i read
      totalBytes += rc;
      // moving my pointer in the char array so i don't overwrite things
      ptr += rc;
    }
    if (flag ==0 ) // if i did a break above because the other side left, i don't want to print anything else
      printf ("received the following '%s'\n", buffer);
    close (connected_sd); // note i close the connected socket, not the one i created and did the bind() to. 

  } // end of while(1) loop



}
