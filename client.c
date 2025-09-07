#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

/* the logic of my program/server
   1: make sure the user started me with the right parameters
   2: create a socket (STREAM/connection oriented)
   3: ask ther user for a filename NOTE: not really doing file IO
   4) figure out length of 'filename'
   5) put the server's address info into my datastructure
   6) connect() to the server
   7) send length of filename
   8) send filename
*/

/*****************************
 ****** GLOBAL VARIABLES *****
 *****************************/

// socket descriptor
int sd;
// socket address object
struct sockaddr_in sin_addr;
// server port
int server_port;
// server IP address string
char* server_ip;



/*****************************
 ********* FUNCTIONS *********
 *****************************/

// closes socket and exits with error
void abort_program(const char* msg) {
    printf(msg);
    close(sd);
    exit(1);
}

// retreives arguments from command line and saves them to global variables
void parse_args(int argc, char *argv[]) {

    // check number of arguments
    if (argc < 3) {
        abort_program("ERROR: Invalid arguments\nUSAGE: client <server port> <server IP address>\n");
    }

    // get the IP address
    server_ip = argv[1];
    if (inet_pton(AF_INET, server_ip, &(sin_addr.sin_addr)) <= 0) {
        abort_program("ERROR: Invalid port number\nUSAGE: client <server port> <server IP address>\n");
    }

    // get the port number
    server_port = atoi(argv[2]);
    if (server_port == 0){
        abort_program("ERROR: Invalid port number\nUSAGE: client <server port> <server IP address>\n");
    }
    sin_addr.sin_port = htons(server_port);

}


// create socket and connect to the server
void server_connect() {

    printf("Connecting to <%s:%i>...\n", server_ip, server_port);

    // create socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("error opening stream socket\n");
    }

    // connect to server
    if (connect(sd, (struct sockaddr*)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
        abort_program("ERROR: Could not connect to server\n");
    }

    printf("Connection successful!\n");

}


// read user input string
int get_user_input(char* in_buf) {

    int user_input_strlen;

    printf("Enter a string to send to the server:\n");

    if (scanf("%[^\n]%*c", in_buf) != 1) {  // had to look up/copy the "%[^\n]%*c" format on stackexchange
        abort_program("ERROR: Unknown input\n");
    }

    user_input_strlen = strlen(in_buf);
    printf("\nRead %i characters from user input", user_input_strlen);

    return user_input_strlen;

}


// sends two messages to server: 1st the user input length, 2nd the user input
void server_send_data(char* user_input_buf, int user_input_buflen) {

    // send buffer length
    printf("Sending user input length to server...\n");
    if (write(sd, &user_input_buflen, sizeof(int)) != 4) { // it should be able to send 4 bytes at once...
        abort_program("ERROR: Could not send user input length\n");
    }
    printf("Successfully sent user input length!\n");

    // send buffer contents
    printf("Sending user input string to server...\n");
    int sent_bytes = 0;
    while (sent_bytes < user_input_buflen) {  // loop until all bytes have been sent
        int buf_offset = user_input_buflen + sent_bytes;
        int remaining_bytes = user_input_buflen - sent_bytes;

        int just_sent_bytes = write(sd, (user_input_buf + buf_offset), remaining_bytes);
        if (just_sent_bytes <= 0) {
            abort_program("ERROR: Could not send user input length\n");
        }
        sent_bytes += just_sent_bytes;
    }
    printf("Successfully sent user input string!\n");

}



/*****************************
 *********** MAIN ************
 *****************************/

int main(int argc, char *argv[])
{

    // populate socket address object
    sin_addr.sin_family = AF_INET;
    parse_args(argc, argv);

    // connect to the server
    server_connect();

    // get user input
    char user_input_buf[256];
    int user_input_buflen = get_user_input(user_input_buf) + 1; // +1 because of null terminator

    // send the user input
    server_send_data(user_input_buf, user_input_buflen);

    // success!
    printf("Client execution successful!\n");
    close(sd);
    return 0;

}
