// AUTHOR: Mark Evers <mark.evers@ucdenver.edu> <mevers303@gmail.com>
// DATE:   9/8/2025
// TITLE:  CSCI 3761: Project 1


/*****************************
 ******** DEPENDENCIES *******
 *****************************/

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>


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

// closes socket and exits with error message
void abort_program(const char* msg) {
    perror(msg);
    close(sd);
    exit(1);
}


// retreives arguments from command line and saves them to global variables
void parse_args(int argc, char *argv[]) {

    // check number of arguments
    if (argc < 3) {
        abort_program("ERROR: Invalid arguments\nUSAGE: client <server port> <server IP address>\n");
    }

    // get the port number
    server_port = atoi(argv[1]);
    if (server_port <= 0){
        abort_program("ERROR: Invalid port number\nUSAGE: client <server port> <server IP address>\n");
    }
    // convert to network short
    sin_addr.sin_port = htons(server_port);

    // get the IP address
    server_ip = argv[2];
    // convert IP address string to network long
    if (inet_pton(AF_INET, server_ip, &(sin_addr.sin_addr)) <= 0) {
        abort_program("ERROR: Invalid port number\nUSAGE: client <server port> <server IP address>\n");
    }

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
    printf("Read %i characters from user input\n", user_input_strlen);

    return user_input_strlen;

}


// sends two messages to server: 1st the user input length, 2nd the user input
void server_send_data(char* user_input_buf, int user_input_buflen) {

    // send buffer length
    printf("Sending user input length to server...\n");
    int nl_user_input_buflen = htonl(user_input_buflen);
    if (write(sd, &nl_user_input_buflen, sizeof(int)) != 4) { // it should be able to send 4 bytes at once...
        abort_program("ERROR: Could not send user input length\n");
    }
    printf("Successfully sent user input length!\n");

    // send buffer contents
    printf("Sending user input string to server...\n");
    int sent_bytes = 0;
    while (sent_bytes < user_input_buflen) {  // loop until all bytes have been sent
        int remaining_bytes = user_input_buflen - sent_bytes;

        int just_sent_bytes = write(sd, (user_input_buf + sent_bytes), remaining_bytes);
        if (just_sent_bytes <= 0) {
            abort_program("ERROR: Could not send user input length\n");
        }
        sent_bytes += just_sent_bytes;
    }
    printf("Successfully sent user input string!\n");

}


// sends two messages to server: 1st the user input length, 2nd the user input
int server_receive_data(char* return_buffer) {

    // server return size
    int return_buffer_size;

    // get buffer length
    int nl_return_buffer_size;
    printf("Retrieving server response length...\n");
    if (read(sd, &nl_return_buffer_size, sizeof(int)) != 4) { // don't loop send, it should be able to send 4 bytes at once...
        abort_program("ERROR: Could not send user input length\n");
    }
    return_buffer_size = ntohl(nl_return_buffer_size);
    printf("Successfully received server response length: %i\n", return_buffer_size);

    // send buffer contents
    printf("Retreiving server response string...\n");
    // loop until all bytes have been sent
    int received_bytes = 0;
    while (received_bytes < return_buffer_size) {
        int remaining_bytes = return_buffer_size - received_bytes;
        int just_received_bytes = read(sd, (return_buffer + received_bytes), remaining_bytes);
        if (just_received_bytes <= 0) {
            abort_program("ERROR: Could not send user input length\n");
        }
        received_bytes += just_received_bytes;
    }
    printf("Successfully received server response string!\n");

    // return the length of the buffer
    return return_buffer_size;

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
    memset(user_input_buf, 0, 256);
    int user_input_buflen = get_user_input(user_input_buf) + 1;  // +1 because of the null terminator

    // send the user input
    server_send_data(user_input_buf, user_input_buflen);

    // get the server response
    char server_response_buf[256];
    int server_response_buflen;
    memset(server_response_buf, 0, 256);
    server_response_buflen = server_receive_data(server_response_buf);
    // print the returned data
    printf("Received %i bytes from server\n", server_response_buflen);
    printf("%s\n", server_response_buf);

    // success!
    printf("Client execution successful!\n");
    close(sd);
    return 0;

}
