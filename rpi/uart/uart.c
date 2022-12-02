#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <wiringPi.h>
#include <wiringSerial.h>



#define BAUD_RATE 9600

// Read message after secret code.
int FLAG_READ = 0;
char key[6] = {'u', 'n', 'l', 'o', 'c', 'k'};

/**
 * Initialize Uart
 */ 
int init_uart(){
    int serial_port = serialOpen("/dev/serial0", BAUD_RATE);
    if (serial_port < 0) {
        fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
        return 0;
    }

    printf("Reading secret code\n");
    
    // Unlock
    int i = 0;
    while(i < 6){
        if (serialDataAvail(serial_port)){
            char byte = serialGetchar(serial_port); /* receive character serially*/
            printf("%d %c\n",byte, byte);
            i = byte == key[i] ? i + 1 : 0;
            fflush(stdout);
        }
    }
    printf("Secret code found\n");
    return serial_port;
}

/**
 * Wait uart response
 */
int busy_wait(int serial_port){
    int t = 0;
    while(!serialDataAvail(serial_port) && t < 10) {
        delay(90); 
        t++;
    }
    if(t >= 10) {
        printf("Timeout...\n");
        return 0;
    }
    return 1;
}

void send_data(int port, int data){
    serialPutchar(port, data);
}

int get_response(int port){
    
    int response = busy_wait(port);
    if(!response) return -1;
    return serialGetchar(port);
}

