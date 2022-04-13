#include "mbed.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "uLCD_4DGL.h"

//macros
#define DATA_SIZE 4096
#define DEFAULT_FREQ 10
#define FIX_PRECISION_PNT 100

//i/o
DigitalInOut test_led(LED1);
InterruptIn A_BUTTON(D5);
InterruptIn B_BUTTON(D6);
AnalogOut aout(PA_4);
AnalogIn  ain(A0);
Ticker tick;

Timer debounce;
uLCD_4DGL uLCD(D1, D0, D2);

//thread& eventqueue
Thread wave_gener_thread;
Thread wave_detect_thread;
Thread task_handle_thread;

EventQueue gener_queue;
EventQueue detect_queue;
EventQueue task_queue;


//global var
int idx = 0;
int idx2 = 0;
double *pntArray;
double *dataArray;
bool flag = true;


//function predef
void start_handler();
void stop_handler();
void setzero();
void cal_pntArray();

void tick_call();

void output_handler();
void measure_handler();
void idle();

void data_return();
void display(int id);

////////////////////////MAIN FUNCTION///////////////////////
int main()
{
    //initailize
    A_BUTTON.mode(PullNone);
    B_BUTTON.mode(PullNone);
    debounce.start();

    wave_gener_thread.start(callback(&gener_queue, &EventQueue::dispatch_forever));
    task_handle_thread.start(callback(&task_queue, &EventQueue::dispatch_forever));
    wave_detect_thread.start(callback(&detect_queue, &EventQueue::dispatch_forever));
    A_BUTTON.rise(&start_handler);
    B_BUTTON.rise(&stop_handler);


    pntArray = (double*)malloc(FIX_PRECISION_PNT * sizeof(double));
    dataArray = (double*)malloc(DATA_SIZE*sizeof(double));
    setzero();
    tick.attach(&tick_call, 1000000us/int(DEFAULT_FREQ*FIX_PRECISION_PNT));
    
    while(1) 
    {
        task_queue.call(display, 1);//to give a starting msg
        
        while(flag)  {ThisThread::sleep_for(100ms);}//flag lock

        task_queue.call(display, 4);//to give a ending msg
        flag = true;
    }

    return 0;
}


//////////////////function def//////////////////
void start_handler()
{
    if(debounce.elapsed_time()>2000ms)
    {
        task_queue.call(cal_pntArray);
        task_queue.call(display, 2);//to give the generate msg
        debounce.reset();
    }
}
    
void stop_handler()
{
    if(debounce.elapsed_time()>2000ms)
    {
        detect_queue.call(idle);
        task_queue.call(setzero);
        task_queue.call(display, 3);
        task_queue.call(data_return);
        debounce.reset();
    }
    
}
void setzero()
{
    for(int i=0; i<FIX_PRECISION_PNT; i++)  pntArray[i] = 0;
    aout = 0;
}
void cal_pntArray(){
    pntArray[0] = 0;
    
    int i=0;
    while(i < FIX_PRECISION_PNT/2){
        pntArray[i+1] = 3.0*tanh(3.0/FIX_PRECISION_PNT/2*i);  i++;
    }while(i < FIX_PRECISION_PNT){
        pntArray[i+1] = 3.0*(1-tanh(3.0/FIX_PRECISION_PNT/2*i-3));  i++;
    }
}


void tick_call()
{
    gener_queue.call(output_handler);
    detect_queue.call(measure_handler);
}


void output_handler()
{
    aout = pntArray[idx % FIX_PRECISION_PNT];
    idx += 1;
}
void measure_handler()
{
    dataArray[idx2 % DATA_SIZE] = ain;
    idx2 += 1;
}
void idle()
{
    idx2 = 0;
    while(flag)  ThisThread::sleep_for(100ms);
}



void data_return()
{
    for(int i=0; i<DATA_SIZE; i++)  printf("%.3f\n", dataArray[i]);
    flag = false;
}
void display(int id)
{
    if(id == 1){
        //program start
        uLCD.printf("*push buttonA\n to start.\n");
    }
    if(id == 2){
        //generate waveform
        uLCD.printf("\n");
        uLCD.printf("*start generating\n");
        uLCD.printf("*push buttonB\n to transfer data\n");
        printf("%f\n",1.0*DEFAULT_FREQ*FIX_PRECISION_PNT);
    }
    if(id == 3){
        //stop generate & collecting data
        uLCD.printf("\n");
        uLCD.printf("*stop generating\n");
        uLCD.printf("*transfering data\n");
    }
    if(id == 4){
        //program end
        uLCD.printf("\n");
        uLCD.printf("*transfer complete\n start a new cycle\n");
        ThisThread::sleep_for(1000ms);
        uLCD.cls();
    }
}



