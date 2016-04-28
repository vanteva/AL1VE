#include <18F2550.h>
#device ADC=10

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES WDT128                   //Watch Dog Timer uses 1:128 Postscale
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOXINST                  //Extended set extension and Indexed Addressing mode disabled (Legacy mode)

#use delay(crystal=20000000)
#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=PORT1)
//#use i2c(Master,Fast,sda=PIN_C4,scl=PIN_C3,force_hw)
//#use i2c(master, sda=PIN_C4, scl=PIN_C3, FAST, FORCE_HW)
#use I2C(master, sda=PIN_B0, scl=PIN_B1)


