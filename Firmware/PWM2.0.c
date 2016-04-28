#include <PWM2.0.h>
#include <math.h>
#include "DHT22drv.c"
#include "BMP180DRV.c"
#include "BMP180DRV.h"

#bit mov_io = 0xf94.0
//#define BMP085_ADDRESS 0xEE          // I2C address of BMP085 
//#define P_CORRECTION   1.5           // in mBars - factor to adjust for elevation to match local weather station pressure 
                                     // this value for 14' above sea level (in Boca Raton, Florida) 


char data[2] = {'^','^'};  //2 chars for sending as data
/*
void LED_high(void)
{
   output_high(pin_b5);
   output_high(pin_b4);
   output_high(pin_b3);
   output_high(pin_b2);
   output_high(pin_b1);
   output_high(pin_b0);
}*/

#int_rda
void WriteValue(void)
{
   disable_interrupts(int_rda);
   gets(data, PORT1);
   //LED_high();
   enable_interrupts(int_rda);
}

void main()
{

   //LED_high();
   enable_interrupts(int_rda);
   enable_interrupts(global);

   setup_timer_2(T2_DIV_BY_1,249,1);      //50.0 us overflow, 50.0 us interrupt
   SET_TRIS_B(0x00);
   //mov_io = 0;
   //output_high(pin_c0);
   //mov_io = 1;
   SET_TRIS_C(0x81);
   //SET_TRIS_A(0x00);
   setup_ccp1(CCP_PWM);
   setup_ccp2(CCP_PWM);
   set_pwm1_duty((int16)0);
   set_pwm2_duty((int16)0);
   
   //DHT22 sensor
   float dhthum, dhttem;
   int1 minus = 0;
   
   //distance sensor
   unsigned int16 voltage;
   setup_adc_ports(AN0);
   setup_adc(ADC_CLOCK_INTERNAL);
   //set_adc_channel(0);
   
   //movement sensor
   //int8 mov = 0;
   int8 cycles = 0;
   
   //soil sensor
   //setup_adc_ports(AN1);
   //setup_adc(ADC_CLOCK_INTERNAL);
   //set_adc_channel(0);
   
   //pressure sensor
   port_B_pullups(0x03);
   //float Tf, P_inHg; 
   float T_Cent; 
   float P_mBar; 
   //int16 tp, tt; 
   bmp085Calibration();
   
   //while(!kbhit()); //continue after a key is pressed 
   delay_ms(20000);
   while(TRUE)
   {
      cycles++;
      
      //gets(data,PORT1); // reads 2 chars
      //printf(data); // echo
      int16 L = data[0];  // left
      int16 R = data[1];  // right
      //printf("%c %c\n\r",data[0],data[1]);
      // result values
      int16 Lpwm = 0, Rpwm = 0;
     
      // control left
      if(L >= 97)
      {
         Lpwm = (L - 97)*32;
         output_low(pin_b7);     //close pin_b7
      }
      if(L <= 89)
      {
         Lpwm = (L - 65)*32;        
         output_high(pin_b7);    //open pin_b7
      }
      // conrol right
      if(R >= 97)
      {
         Rpwm = (R - 97)*32;
         output_low(pin_b6);     //close pin_b6
      }
      if(R <= 89)
      {
         Rpwm = (R - 65)*32;
         output_high(pin_b6);    //open pin_b6
      }  
      
      //setup pwm
      set_pwm1_duty(Lpwm);
      set_pwm2_duty(Rpwm);
      //delay_us(500);
      
      /*
      //BLINKING LEDs
      if(L>R) //right
      {
         output_toggle(pin_b2);
         output_toggle(pin_b0);
         delay_ms(250);
      }
      else if(L<R) //left
      {
         output_toggle(pin_b5);
         output_toggle(pin_b3);
         delay_ms(250);
      }
      else if(L<89 && R<89 && L==R) // straight backwards
      {
         output_toggle(pin_b5);
         output_toggle(pin_b2);
         delay_ms(250);
      }
      else // nothing blinks
      {
         LED_high();
      }
      */
   
      //Temperature and humidity sensor
      read_dht22 (dhthum, dhttem, minus);
      delay_ms(10);
      if(minus == 1)
      {
         printf("T %3.1f %2.1f ", dhthum, -dhttem);
         minus = 0;
      }
      else
      {
         printf("T %3.1f %2.1f ", dhthum, dhttem);
      }
      delay_ms(2000);
   
      //distance sensor
      set_adc_channel(0);
      delay_us(20);
      for(int i=0; i<5; i++)
      {
         voltage += read_adc();
      }
      voltage /= 5; //*0.0048828125
      printf("P %lu ", voltage);
      
      //movement sensor
      /*
      if(cycles == 6)
      {
         cycles=0;
         if(input(pin_c0) == 0)
         {
            //printf("input(pin_c0) = 0\n\r");
            //mov++;
            //if(mov >= 3)
            //{
               printf("Something moves!\n\r");
               while(input(pin_c0) == 0)
               {
                  set_pwm1_duty(0);
                  set_pwm2_duty(0);
                  delay_ms(1500);
               }
            //mov = 0;
          }
         else
         {
            //if(mov != 0)
            //{
            //   mov = 0;
            //}
            printf("input(pin_c0) = 1\n\r");
         }
      }*/
      
      //soil sensor
      //set_adc_channel(1);
      //delay_us(20);
      //int8 sens_value = read_adc();
      //printf("Soil INFO: %u", sens_value);
      
      //presure sensor

      T_Cent = BMP085Temperature(); 
      //tt= get_timer0(); 
      P_mBar = BMP085Pressure(false);   // skips required temp reading since already done above
      //tp = get_timer0() - tt;
      //printf("Temperature (C):  %.1g\n\r", T_Cent);
      printf("Pressure %0.2g ", P_mBar);
      //Tf = ((9.0/5.0)*T_Cent) + 32.0; 
      //printf("Temperature (F):  %.1g\r\n", Tf); 
      //P_inHg = (0.0295301) * P_mBar; 
      //printf("Pressure (inHg):  %.2g\r\n\n", P_inHg); 
      //printf("Processing time: \r\n"); 
      //printf("  Temperature = %Lu  (usec)\r\n", tt); 
      //printf("  Pressuure   = %Lu (usec)\r\n", tp); 
      //printf("------------------------------\r\n"); 
   }
}
