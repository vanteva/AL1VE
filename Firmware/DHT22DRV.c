//Driver DHT22

#define dht22 PIN_A2
#bit dht_io = 0xf92.2         // TRISA register bit 2

byte dht22_dat[5];            // 5 bytes of data
unsigned int8 check;
byte read_data_dht();        // read data from dht22

void dht_init (void)
{
   dht_io=0;               // 
   output_low(dht22);
   delay_ms(18);
   output_high(dht22);
   delay_us(30);
   dht_io = 1;
}
void CheckRe()
{
   check = 0;
   delay_us(40);
   if(input(dht22) == 0)
   {
      delay_us(80);
      if(input(dht22) == 1) check = 1;
      delay_us(40);
   }
}
byte read_data_dht()
{
   byte i = 0;
   byte result=0;
   for (i=0; i<8; i++) 
   {
      while(input(dht22) == 0);
      delay_us(30);
      if (input(dht22) == 0) result &= ~(1<<(7-i));
      else 
      {
         result |=(1<<(7-i));
         while (input(dht22)==1);
      }
   }
   return result;
}
void read_dht22 (float &dhthum, float &dthtemp, int1 &minus)
{
   byte i, dht22_checksum;
   int16 temperature, humidity;
   float temp, hum;
   dht_init();
   CheckRe();
   if(check == 1)
   {
      for (i=0; i<5; i++)
      {
         dht22_dat[i] = read_data_dht();
      }
   }
   dht22_checksum = dht22_dat[0]+dht22_dat[1]+dht22_dat[2]+dht22_dat[3];
   if(dht22_dat[4] != dht22_checksum)
   {
      printf("DHT checksum error");
   }
   humidity = make16(dht22_dat[0], dht22_dat[1]);
   temperature = make16(dht22_dat[2], dht22_dat[3]);
   if(temperature > 0x8000)
   {
      temperature = temperature & 0x7FFF;
      minus = 1;
   }
   hum = humidity;
   temp = temperature;
   dhthum = (hum)/10;
   dthtemp = (temp)/10;
}
