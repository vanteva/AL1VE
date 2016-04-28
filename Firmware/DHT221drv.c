#bit dht_io = 0xf92.2

void DHT_Start(void) 
{ 
   // Here we send the 'start' sequence to the DHT sensor! We pull the DHT_IO pin low for 25mS, and 
   // then high for 30mS. The DHT sensor then responds by pulling the DHT_IO pin low, and then pulling it 
   // back high. 
   dht_io=0;
   printf("In DHT_Start()\n\r");
   output_low(pin_a2);
   printf("after output_low\n\r");
   delay_ms(20); // wait at least 1ms at low
   output_high(pin_a2);
   printf("After output_high\n\r");
   delay_us(40); // wait 40us at high
   
   dht_io = 1; //makes pin_a2 an intput
   printf("Made a2 input\n\r");
   while(input(pin_a2) == 0);
   printf("got low at a2\n\r");
   while(input(pin_a2) == 1);
   printf("got high at a2\n\r");
} 

int8 DHT_ReadData(void) 
{ 
    
   // This subroutine reads a byte of data (8 bits) from the DHT sensor... 
    
   int8 result = 0; // 

   for(byte i=0; i< 8; i++){ // 

    while(input(pin_a2) == 0); // 
    delay_us(30);               // 
    if (input(pin_a2) == 1)   // 
      result |= (1 << (7 - i));        // 
                                           //                
    while (input(pin_a2)== 1); // 
   
   return result;
}
}

void DHT_MakeData(float *dhthum, float *dhttem)
{
   byte iByteIndex, check_sum, dht_data[5];
   
    // Here we read a total of 5 bytes of data (40 bits) from the DHT sensor. For the DHT-22 sensor, the integer data is  
    // is contained in the 1st (Humidity), and 3rd (Temperature) bytes transmitted, and the decimal 
    // data is contained in the 2nd (Humidity), and 4th (Temperature) bytes transmitted. The 5th byte 
    // contains the CRC byte for both sensor types.
   
   for(iByteIndex = 0; iByteIndex < 5 ; iByteIndex++) 
   { 
      dht_data[iByteIndex] = DHT_ReadData(); 
   }

    // Here we calculate the check_sum by adding the 1st four bytes, and looking at only the lower 8 bits. 
    // This value should match the 5th byte sent by the sensor which is the check_sum byte! 
    
    check_sum = dht_data[0] + dht_data[1] + dht_data[2] + dht_data[3]; 
    //check_sum = check_sum & 0xFF; 

    if (check_sum != dht_data[4]) 
      printf("\n\rcheck_sum error!\n");
   
   int16 Humidity = make16(dht_data[0], dht_data[1]); 
   int16 Temperature = make16(dht_data[2] & 0x7F, dht_data[3]);   //Strip off the sign bit first! 
   
    if (dht_data[2] & 0x80) 
        Temperature *= -1;

   dhthum = Humidity/10;
   dhttem = Temperature/10;
}
