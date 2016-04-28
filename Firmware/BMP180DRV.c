
//************************************************ 
//  BMP085 Barometric Pressure Sensor 
// 
//  - Datasheet: http://www.bosch-sensortec.com/content/language1/downloads/BST-BMP085-DS000-05.pdf 
// 
//  - Written in CCS PCH C using floating point math 
//  - Several integer math versions of this driver exist but the speed improvement is 
//    not warranted in typical weather station type applications 
//  
//  - Based on a paper posted to thebackshed.com by DuinoMiteMegaAn 
//    http://www.thebackshed.com/forum/forum_posts.asp?TID=4768&PN=9  
// 
//  - Usage: 
//     Call once: bmp085Calibration(); 
//     P_mBar_float = BMP085Pressure(true);  //  calls for temperature first 
//     P_mBar_float = BMP085Pressure(false);  // skips temperature reading, assumes done previously 
//     T_Cent_float = BMP085Temperature(); 
//     t_reading = _Temp;  _Temp set on every temperature reading 
//        Note:   pressure reading is temp compensated so call for temp reading prior to pressure reading periodically or on each reading 
// 
//  Al Testani 
//  08/17/12 
//************************************************ 

// place a #use i2c statement in the main program and comment this out if not applicable 
//#use i2c(master, sda=PIN_C4, scl=PIN_C3, FAST, FORCE_HW) 

#include <math.h>

const int8 OVS_S = 3; // Oversampling Setting (0,1,2,3 from ultra low power, to ultra hi-resolution) 

#define BMP085_ADDRESS 0xEE          // I2C address of BMP085 
#define P_CORRECTION   1.5           // in mBars - factor to adjust for elevation to match local weather station pressure 
                                     // this value for 14' above sea level (in Boca Raton, Florida) 


// Calibration values 
signed int16 ac1; 
signed int16 ac2; 
signed int16 ac3; 
int16 ac4; 
int16 ac5; 
int16 ac6; 
signed int16 b1; 
signed int16 b2; 
signed int16 mb; 
signed int16 mc; 
signed int16 md; 

// floating point cal factors 
float _c3; 
float _c4; 
float _b1; 
float _c5; 
float _c6; 
float _mc; 
float _md; 

// polynomomial constants 
float _x0; 
float _x1; 
float _x2; 
float _y0; 
float _y1; 
float _y2; 
float _p0; 
float _p1; 
float _p2; 

float _s;     // T-25, used in pressure calculation - must run temperature reading before pressure reading 
float _Temp;  // set after every temperature or temperature/pressure reading 


//---------------------------------------------- 
int8 BMP085ReadByte(int8 address) 
//---------------------------------------------- 
{ 

   int8 data;
   i2c_start();
   i2c_write(BMP085_ADDRESS);
   i2c_write(address);
   i2c_start();
   i2c_write(BMP085_ADDRESS | 0x01);
   data = i2c_read(0);
   i2c_stop();
   return(data);
   
/*   int8 data;
   i2c_start(); 
   i2c_write(BMP085_ADDRESS); 
   i2c_write(address); 
   i2c_start(); 
   i2c_write(BMP085_ADDRESS | 0x01 ); 
   data=i2c_read(0); 
   i2c_stop(); 
   return(data); */
} 


//---------------------------------------------- 
int16 BMP085ReadInt(int8 address) 
//---------------------------------------------- 
{ 
int8 msb, lsb; 
int16 temp; 

   i2c_start(); 
   i2c_write(BMP085_ADDRESS); 
   i2c_write(address); 
   i2c_start(); 
   i2c_write(BMP085_ADDRESS | 0x01 ); 
   msb = i2c_read(); 
   lsb = i2c_read(0); 
   i2c_stop(); 
   temp = make16(msb, lsb); 
   return ( temp ); 
} 


//---------------------------------------------- 
void BMP085WriteByte(int8 address, int8 data) 
//---------------------------------------------- 
{ 
   i2c_start(); 
   i2c_write(BMP085_ADDRESS);
   i2c_write(address); 
   i2c_write(data); 
   i2c_stop(); 
} 


//---------------------------------------------- 
void bmp085Calibration() 
//---------------------------------------------- 
{ 
   // read BMP085 EEPROM cal factors 
   ac1 = bmp085ReadInt(0xAA); 
   ac2 = bmp085ReadInt(0xAC); 
   ac3 = bmp085ReadInt(0xAE); 
   ac4 = bmp085ReadInt(0xB0); 
   ac5 = bmp085ReadInt(0xB2); 
   ac6 = bmp085ReadInt(0xB4); 
   b1  = bmp085ReadInt(0xB6); 
   b2  = bmp085ReadInt(0xB8); 
   mb  = bmp085ReadInt(0xBA); 
   mc  = bmp085ReadInt(0xBC); 
   md  = bmp085ReadInt(0xBE); 

    // calculate floating point cal factors 
   _c3 = 0.0048828125 * ac3;            // 160 * pow2(-15) * ac3; 
   _c4 = 0.000000030517578125 * ac4;    // 1E-3 * pow2(-15) * ac4; 
   _c5 = 0.00000019073486328125 * ac5;  // (pow2(-15)/160) * ac5; 
   _c6 = (float)ac6; 
   _b1 = 0.00002384185791015625 * b1;   // 25600 * pow2(-30) * b1; 
   _mc = 0.08 * mc;                     // (pow2(11) / 25600) * mc; 
   _md = (float)md / 160; 
    
   // calculate polynomial constants 
   _x0 = (float)ac1; 
   _x1 = 0.01953125 * ac2;             // 160 * pow2(-13) * ac2; 
   _x2 = 0.000762939453125 * b2;       // 25600 * pow2(-25) * b2; 
   _y0 = _c4 * 32768;                  //_c4 * pow2(15); 
   _y1 = _c4 * _c3; 
   _y2 = _c4 * _b1; 
   _p0 = 2.364375;                     // (3791-8)/1600
   _p1 = 0.992984;                     // 1-7357*2^(-20)
   _p2 = 0.000004421;                  // 4.420871846377849578857421875e-6     ????  3038*100*2^(-36)
} 


// Read the uncompensated temperature value 
//---------------------------------------------- 
int16 BMP085ReadUT() 
//---------------------------------------------- 
{ 
int16 ut; 
  
  // Write 0x2E into Register 0xF4 
  BMP085WriteByte(0xF4, 0x2E); 
  delay_ms(5); // Wait at least 4.5ms 
  // Read two bytes from registers 0xF6 and 0xF7 
  ut = BMP085ReadInt(0xF6); 
  return((float)ut); 
} 


// Read the uncompensated pressure value 
//---------------------------------------------- 
int32 bmp085ReadUP() 
//---------------------------------------------- 
{ 
int8 msb, lsb, xlsb; 
float p; 
  
  // Write 0x34+(OSS<<6) into register 0xF4 
  // Request a pressure reading w/ oversampling setting 
  BMP085WriteByte(0xF4, (0x34 + (OVS_S<<6)) ); 
  
  // Wait for conversion, delay time dependent on OSS 
  switch (OVS_S) 
  { 
     case 0: delay_ms(5);  break; 
     case 1: delay_ms(8);  break; 
     case 2: delay_ms(14); break; 
     case 3: delay_ms(26); break; 
  }    
  
  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB) 
   msb  = BMP085ReadByte(0xF6); 
   lsb  = BMP085ReadByte(0xF7); 
   xlsb = BMP085ReadByte(0xF8); 
   p = (256*msb) + lsb + (xlsb/256); 
   return(p); 
} 


//---------------------------------------------- 
float BMP085GetTemp(float _tu) 
//---------------------------------------------- 
{ 
float alpha, T; 

   alpha = _c5 * (_tu - _c6); 
   T = alpha + (_mc/(alpha + _md)); 
   _s = T - 25; 
   return(T); 
}    


//---------------------------------------------- 
float BMP085GetPressure(float _pu) 
//---------------------------------------------- 
{ 
float x, y, z; 
float P; 

   x = _x2*_s*_s + _x1*_s + _x0; 
   y = _y2*_s*_s + _y1*_s + _y0; 
   z = ((float)_pu - x) / y; 
   P = _p2*z*z + _p1*z + _p0; 
   P += P_CORRECTION; 
   return(P); 
} 


//---------------------------------------------- 
float BMP085Pressure(boolean getTemp) 
//---------------------------------------------- 
{ 
   if (getTemp) 
      _Temp = BMP085GetTemp(BMP085ReadUT());  // creates _s required for pressure calculation 
   return(BMP085GetPressure(BMP085ReadUP())); 
} 


//---------------------------------------------- 
float BMP085Temperature(void) 
//---------------------------------------------- 
{ 
   _Temp = BMP085GetTemp(BMP085ReadUT()); 
   return(_Temp); 
} 




//A test program using the driver follows: 

//Code:

//#include <18LF2420.h> 
//#device adc=16 
//#FUSES NOWDT, WDT128, H4, NOFCMEN, NOIESO, NOBROWNOUT, NOPBADEN, NOLPT1OSC, NOSTVREN, NOLVP, NOXINST // 4MHz crystal 
//#use delay(clock=16000000) 
//#use rs232(baud=57600, xmit=PIN_C6, rcv=PIN_C7, bits=8, parity=N, stop=1, errors) 

//#include "./BMP085.c" 


// Pin Definitions 
//#define YEL_LED         PIN_B3 
//#define SCL             PIN_C3  
//#define SDA             PIN_C4  
//#define TX              PIN_C6 
//#define RX              PIN_C7 

/*
// Macros 
#define set(x) output_high(x) 
#define clr(x) output_low(x) 


//---------------------------------------------- 
void main() 
//---------------------------------------------- 
{ 
float Tf, P_inHg; 
float T_Cent; 
float P_mBar; 
int16 tp, tt; 

   setup_timer_0(T0_INTERNAL|T0_DIV_4); // 1 usec resolution 
   bmp085Calibration(); 

   while(true) 
   { 
      set(YEL_LED); 
      set_timer0(0); 
      
      T_Cent = BMP085Temperature(); 
      tt= get_timer0(); 
      P_mBar = BMP085Pressure(false);   // skips required temp reading since already done above 
      tp = get_timer0() - tt; 
      printf("Temperature (C):  %.1g\r\n", T_Cent); 
      printf("Pressure (mBar):  %0.2g\r\n\n", P_mBar); 
      Tf = ((9.0/5.0)*T_Cent) + 32.0; 
      printf("Temperature (F):  %.1g\r\n", Tf); 
      P_inHg = (0.0295301) * P_mBar; 
      printf("Pressure (inHg):  %.2g\r\n\n", P_inHg); 
      printf("Processing time: \r\n"); 
      printf("  Temperature = %Lu  (usec)\r\n", tt); 
      printf("  Pressuure   = %Lu (usec)\r\n", tp); 
      printf("------------------------------\r\n"); 
      
      clr(YEL_LED); 
      delay_ms(1000); 
   } 
} */
