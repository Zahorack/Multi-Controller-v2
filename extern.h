#ifndef EXTERN_H_
#define EXTERN_H_

#include <Arduino.h>

/* Some maths for battery */
#define   MINVOLT         3.7
#define   UrefCorection   0.932             /* 4.66/5 */
#define   Uref            5
#define   Ulow            3.7               /*for Lion baterries discharged state*/
#define   Uhigh           4.2               /*for Lion baterries fullcharged state*/
#define   lower(Ur,Ul)    (Ul* 1024/ Ur)    /*3.3 V   725*/
#define   upper(Ur,Uh)    (Uh* 1024/ Ur)    /*4.2 V  923*/

extern "C" {
        
extern const uint8_t BATTERY_PIN;
extern uint8_t getBatteryLevel();
extern void updateBattery();

extern int clamp(int val, int min, int max);

extern void Trace(String);

}
#endif
