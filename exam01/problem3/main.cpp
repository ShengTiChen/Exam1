#include "mbed.h"
#include "stm32l475e_iot01_accelero.h"

InterruptIn btnRecord(BUTTON1);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;

int16_t pDataXYZ[3] = {0}; //vec_a
int16_t vec_b[3];
double angle;

int main() {
   
    BSP_ACCELERO_Init();
    t.start(callback(&queue, &EventQueue::dispatch_forever));

    BSP_ACCELERO_AccGetXYZ(pDataXYZ);
    vec_b[0] = pDataXYZ[0];
    vec_b[1] = pDataXYZ[1];
    vec_b[2] = pDataXYZ[2];



    btnRecord.fall(queue.event(startRecord));
    btnRecord.rise(queue.event(stopRecord));
}




void record(void) {
    BSP_ACCELERO_AccGetXYZ(pDataXYZ);
    //printf("%d, %d, %d\n", pDataXYZ[0], pDataXYZ[1], pDataXYZ[2]);
    double len_a = sqrt(pDataXYZ[0]*pDataXYZ[0]+ pDataXYZ[1]* pDataXYZ[1]+pDataXYZ[2]*pDataXYZ[2]);
    double len_b = sqrt(vec_b[0]*vec_b[0]+vec_b[1]*vec_b[1]+vec_b[2]*vec_b[2]);
    double a_dot_b = pDataXYZ[0]*vec_b[0]+pDataXYZ[1]*vec_b[1]+pDataXYZ[2]*vec_b[2];
    angle = acos(a_dot_b/(len_a*len_b));
    printf("%.1f", angle);
}
/*
void startRecord(void) {
   printf("---start---\n");
   idR[indexR++] = queue.call_every(1ms, record);
   indexR = indexR % 32;
}

void stopRecord(void) {
   printf("---stop---\n");
   for (auto &i : idR)
      queue.cancel(i);
}
*/

