#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <zmq.h>
#include <pthread.h>

#include "data.h"
#include "instruction.h"
#include "rung.h"
#include "plclib.h"
#include "plcemu.h"
#include "config.h"
#include "ui.h"

/*************GLOBALS************************************************/

char * Cli_buf = NULL; 
char * Response_buf = NULL;
char * Update_buf = NULL;

pthread_t Reader;

void * read_cli(void * sock) {
    
    size_t l = 0;
    char * b = NULL;
    
    config_t command = copy_sequences(init_config(), ui_init_command());
    while(getline((char **)&b, &l, stdin)>=0){
        command = cli_parse(b, command);
        char * serialized = serialize_config(command);
        printf("Sending... \n%s\n", serialized);
        zmq_send (sock, 
                  serialized, 
                  strlen(serialized),
                  0);
        zmq_recv (sock, 
                    Response_buf, 
                    CONF_STR, 
                    0); 
        printf("Response:\n %s\n", Response_buf);                     
        
        free(b);
        free(serialized);
    }
    return NULL;
}

//  UI client

int main (void)
{    
    Cli_buf = (char*)malloc(CONF_STR);
    Response_buf = (char*)malloc(CONF_STR);
    Update_buf = (char*)malloc(CONF_STR);
    memset(Cli_buf, 0, CONF_STR);

    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    
    printf ("Connecting to PLC EMU...\n");
    zmq_connect (requester, "tcp://localhost:5555");
    zmq_connect (subscriber, "tcp://localhost:5556");
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, "---", 3);
    pthread_create(&Reader, NULL, read_cli, requester);
    
    for(;;){
        if(zmq_recv (subscriber, 
                    Update_buf, 
                    CONF_STR, 
                    ZMQ_DONTWAIT)>=0){ 
            printf("Update:\n %s\n", Update_buf);
        }
    }
    zmq_close (subscriber);
    zmq_close (requester);
    zmq_ctx_destroy (context);
    
    return 0;
}