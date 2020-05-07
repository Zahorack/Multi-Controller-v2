#include "extern.h"

extern "C" {
        
volatile uint8_t g_boatBatteryLevel;
volatile uint32_t g_singleBeamEcho;

namespace Battery {
        static float KalmanFilter(float z_measured) {
            static float x_est_last = 0;
            static float P_last = 0;
            float K;
            float P;
            float P_temp;
            float x_temp_est;
            float x_est;
            float Q = 0.025;
            float R = 0.7;
        
            x_temp_est = x_est_last;
            P_temp = P_last + Q;
        
            K = P_temp * (1.0/(P_temp + R));
        
            x_est = x_temp_est + K * ((float)z_measured - x_temp_est);
            P = (1- K) * P_temp;
        
            P_last = P;
            x_est_last = x_est;
        
            return (float)x_est;
        }
}

void updateBattery()
{
        static uint32_t last_time = millis();

        if(millis() > (last_time + 500)) {
              getBatteryLevel();  
              last_time = millis();
        }
}

uint8_t getBatteryLevel()
{
        uint16_t voltage= (uint16_t)Battery::KalmanFilter(analogRead(BATTERY_PIN));
        
        voltage = voltage - lower(Uref,Ulow);
        uint8_t charge_level = map(voltage, 0, upper(Uref,Uhigh)-lower(Uref,Ulow), 0, 99);
        //Serial.println(charge_level);
        return charge_level;
}

int clamp(int val, int min, int max)
{
        return val < min ? min : (val > max ? max : val); 
}

void Trace(String s){
	#ifdef TRACE
	
	Serial.println(s);
	
	#endif
}


}
