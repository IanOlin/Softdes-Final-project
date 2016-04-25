/* Carl Moser and Ian Paul
*  Spring 2016 Software Design
*
*  TODO:
*  Put the socket in a class
*  Have pulse be the main thing in this file, addons like the socket and visualizations should be in classes
*/

//includes for printing/closing properly
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <signal.h>

//include for pulse
#include <pulse/simple.h>

#include <mgl2/mgl.h>

//includes for socket
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>

//include for my own header file
#include "main.h"

//include fftw to do the fourier transform
#include <fftw3.h>

//defining variables
#define BUFFER_SIZE 44100/240 //This smaller buffer works better, effectively 240 updates/s
#define excess 128

//setting up global variables

//variable for closing things before quiting
volatile sig_atomic_t flag = 0;

//variables for Pulseaudio
pa_simple *s;
pa_sample_spec ss;

//variables for the lights
char redbluegreen[3] = {char(0), char(0), char(0)};
char leave[3] = {char(0), char(0), char(0)};
int avg;
int absolute;



//variables for the UDP client
int sock;
struct sockaddr_in myaddr;

int main()
{
    mglGraph gr;
    // Initializeing variables
    signal(SIGINT, stop);

    // Setting up UDP client
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(8000);
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET,"0.0.0.0",&myaddr.sin_addr.s_addr);

    // Setting up Pulse
    ss.format = PA_SAMPLE_U8;
    ss.channels = 1;
    ss.rate = 44100;
    s = pa_simple_new(NULL,
                    "Peak",
                    PA_STREAM_RECORD,
                    "alsa_output.pci-0000_00_1b.0.analog-stereo.monitor",
                    "Recording",
                    &ss,
                    NULL,
                    NULL,
                    NULL
                    );

    // Socket setup
    if((sock=socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        printf("%s", "Failed to create socket");
        leaving();
        exit(EXIT_FAILURE);
    }

    // Function calls
    fourier_loop();
    leaving();
    return 0;
}

void stop(int sig){
    //this function changes flag if ctrl+c is pressed
    flag = 1;
}

void leaving(){
    //this function closes all of the open connections
    if(s){
        sendto(sock, leave,sizeof(leave), 0, (struct sockaddr *)&myaddr, sizeof(myaddr));
        close(sock);
        pa_simple_free(s);
        std::cout << "disconnected" << std::endl;
    }
}

void loop(){
    while(true){
        uint8_t buf[BUFFER_SIZE];
        pa_simple_read(s, buf, sizeof(buf), NULL);
        avg = 0;
        for(int i = 0; i < sizeof(buf); i ++){
            absolute = abs(int(buf[i] - excess));
            /*uncomment this for printing

            for(int d = 0; d < b; d++){
                printf(">");
            }
            printf("\n");*/
            avg = avg+absolute;
        }
        avg = avg/sizeof(buf);
        printf("\n");
        printf("%d", absolute);

        //for more of a VU, send avg, for more rapid changes, send intermediate
        vu(avg);
        if(flag){
            break;
        }
    }
}

void fourier_loop(){
    while(true){
        uint8_t buf[BUFFER_SIZE];
        pa_simple_read(s, buf, sizeof(buf), NULL);
        int N = sizeof(buf);
        fftw_complex *in, *out;
        fftw_plan p;

        in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
        out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
        p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

        fftw_execute(p); /* repeat as needed */

        fftw_destroy_plan(p);
        fftw_free(in); fftw_free(out);


        if(flag){
            break;
        }
    }
}

void vu(int level){
    /*this function splits up the value and sends it to the server

    RGB green = (0,255,0)
    RGB yellow = (255,255,0)
    RGB red = (255,0,0)
    */
    if(level<64){
        redbluegreen[1] = char(level*4+3);
        redbluegreen[0] = char(255);
    }
    else{
        redbluegreen[1] = char(255);;
        redbluegreen[0] = char(abs(255 - level*2));
    }
    sendto(sock, redbluegreen ,sizeof(redbluegreen), 0, (struct sockaddr *)&myaddr, sizeof(myaddr));
}
