#include "uart/uart.c"
#include <string.h>
extern void init();
extern void write_char(char ch);
extern void clear_display();

#define MODE_SENSOR 0
#define MODE_FREQUENCY 1
#define MODE_MCU_STATUS 2
#define MODE_SENSOR_STATUS 3

#define TOTAL_SENSOR 3
#define CMD_READ 0b00000011
int current_sensor = 0;
int current_screen = MODE_SENSOR;
int current_frequency = 5;
int changed = 1;

int get_mcu_status_code(){
    return 1;
}

int get_sensor_status_code(int sensor_id){
    return (sensor_id<<3) + 2;
}

int get_read_sensor_code(int sensor_id){
    return (sensor_id << 3) + 3;
}

int get_set_frequency_code(int frequency){
    return (frequency << 3) + 4;
}

void printlcd(char *str1, char *str2, int n, int m){
    clear_display();
    for(int i=0;i<n;i++) write_char(str1[i]);
    for(int i=0;i<40-n;i++) write_char(' ');
    for(int i=0;i<m;i++) write_char(str2[i]);
}


int main(){
    printf("Initializing Display...\n");
    init();
    if (wiringPiSetupGpio() == -1) {
        fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
        return 0;
    }
    printf("Initializing Uart...\n");
    int port = init_uart();
    pinMode(5, INPUT);
    pinMode(19, INPUT);
    pinMode(26, INPUT);
    int active = HIGH;
    int mcu_status = -1;
    int sensor_status = -1;
    int sensor_value = -1;
    int freq_value = -1;
    while(1){
        if (digitalRead(5) == active) {
            current_screen = (current_screen + 1)%4;
            changed = 1;
            delay(500);
        }
        //printf("Screen: %d\n",current_screen);
        if(current_screen == MODE_SENSOR) {
            if (digitalRead(19) == active) current_sensor = (current_sensor-1+TOTAL_SENSOR)%TOTAL_SENSOR, changed=1;
            else if (digitalRead(26) == active) current_sensor = (current_sensor + 1)%TOTAL_SENSOR, changed=1;
            if(changed) delay(50);
            int cmd = get_read_sensor_code(current_sensor);
            send_data(port, cmd);
            int r;
            while((r = get_response(port)) != -1){
                printf("%d %d\n", r, sensor_value);
                if(r != sensor_value || changed){
                    sensor_value = r;
                    changed = 1;
                    clear_display();
                    // Print Header
                    char *msg = "sensor ";
                    for(int i=0;i<7;i++) write_char(msg[i]);
                            
                    write_char('0' + (cmd>>3)/10);
                    write_char('0' + (cmd>>3)%10);
                    for(int i=0;i<31;i++) write_char(' ');
                    // Print body
                    msg = "valor ";
                    for(int i=0;i<6;i++) write_char(msg[i]);
                }
                int c[4] = {};
                for(int i=3;i>=0;i--){
                    c[i] = r%10;
                    r /= 10;
                }
                int f=0;
                for(int i=0;i<4;i++) if(c[i]) {
                    f = i; break;
                }   
                if(changed){
                    changed = 0;
                    for(int i=f; i<4 ;i++) 
                        write_char(c[i]+'0');
                }

                break;
            }
        
        } else if(current_screen == MODE_FREQUENCY) {
            if (digitalRead(19) == active) current_frequency = current_frequency == 0 ? 0 : current_frequency-1, changed=1;
            else if (digitalRead(26) == active) current_frequency = current_frequency == 31 ? 31 : current_frequency+1, changed=1;
            if(changed) delay(50);
            int cmd = get_set_frequency_code(current_frequency);
            send_data(port, cmd);
            int r;
            
            while((r = get_response(port)) != -1){
                printf("feq  %d %d %d %d\n", r, freq_value, current_frequency, cmd);
                if(r != freq_value || changed){
                    freq_value = r;
                    changed = 1;
                    clear_display();
                    // Print Header
                    char *msg = "frequencia ";
                    for(int i=0;i<11;i++) write_char(msg[i]);
            
                    for(int i=0;i<29;i++) write_char(' ');
                    // Print body
                    msg = "valor ";
                    for(int i=0;i<6;i++) write_char(msg[i]);
                }
                int c[4] = {};
                for(int i=3;i>=0;i--){
                    c[i] = r%10;
                    r /= 10;
                }
                int f=0;
                for(int i=0;i<4;i++) if(c[i]) {
                    f = i; break;
                }
                if(changed){
                    changed = 0;
                    for(int i=f; i<4 ;i++) 
                        write_char(c[i]+'0');
                }
                break;
            }
        
        } else if(current_screen == MODE_MCU_STATUS) {
            int cmd = get_mcu_status_code();
            send_data(port, cmd);
            int r;
            while((r = get_response(port)) != -1){
                if(changed || r != mcu_status ){
                    changed = 0;
                    mcu_status = r;
                    char *s;
                    if(r) s = "normal";
                    else s = "com problema";
                    printlcd("nodemcu estado", s, 14, strlen(s));
                }
                break;
            }
        } else if(current_screen == MODE_SENSOR_STATUS) {
            if (digitalRead(19) == active) current_sensor = (current_sensor-1+TOTAL_SENSOR)%TOTAL_SENSOR, changed=1;
            else if (digitalRead(26) == active) current_sensor = (current_sensor + 1)%TOTAL_SENSOR, changed=1;
            if(changed) delay(50);
            int cmd = get_sensor_status_code(current_sensor);
            send_data(port, cmd);
            int r;
            while((r = get_response(port)) != -1){
                if(changed || r != sensor_status ){
                    changed = 0;
                    sensor_status = r;
                    char *s;
                    if(r) s = "normal";
                    else s = "com problema";
                    char s1[17] = "sensor status ";
                    s1[14] = '0' + (current_sensor/10);
                    s1[15] = '0' + (current_sensor%10);
                    s1[16] = '\0';
                    printlcd(s1, s, 17, strlen(s));
                }
                break;
            }
        }
        
        
    }
}