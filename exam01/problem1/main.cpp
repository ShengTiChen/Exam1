#include "mbed.h"
#include "uLCD_4DGL.h"
using namespace std::chrono;
DigitalOut led1(LED1);

InterruptIn button(BUTTON1);
uLCD_4DGL uLCD(D1, D0, D2);
Ticker tick;
Timer debounce;

Thread display_thread;
Thread task_thread;
EventQueue display_queue;
EventQueue task_queue;

int pros_num = 0;
float time_now = 0;
float time_rate;


void trigger();
void pros();
void mode1();
void mode2();
void mode3();
void tick_call();
void add_time();
void update();

// main() runs in its own thread in the OS
int main()
{
    uLCD.cls();
    
    button.mode(PullNone);
    display_thread.start(callback(&display_queue, &EventQueue::dispatch_forever));
    task_thread.start(callback(&task_queue, &EventQueue::dispatch_forever));
    tick.attach(&tick_call, 100ms);

    debounce.start();
    button.rise(&trigger);
    
    while(1)
    {
        
    }
}



void trigger()
{
    if(debounce.elapsed_time()>2000ms)
    {
        task_queue.call(pros);
        debounce.reset();
    }
    //task_queue.call(pros);
}

void pros()
{
    if(pros_num%3 == 0) task_queue.call(mode1);
    if(pros_num%3 == 1) task_queue.call(mode2);
    if(pros_num%3 == 2) task_queue.call(mode3);
    pros_num += 1;
}

void tick_call()
{
    task_queue.call(add_time);
    display_queue.call(update);
}
void add_time()
{
    time_now += time_rate;
}
void update()
{
    //uLCD.cls();
    uLCD.locate(0, 0);
    uLCD.printf("%.3f s\n", time_now);
}


void mode1()
{
    time_rate = 0.05;
}
void mode2()
{
    time_rate = 0;
}
void mode3()
{
    time_now = 0;
}


