# Mobile Weather Station
# AL1VE - MWS


All the files are from a project that is suppose to be a mobile robot-weather station, which sends the data it collects to a universal windows application. They are stored there and visualized in charts. The information can be easily analized and made into a weather forecast. The information gathered is for temperature, humidity and air pressure parameters.

The windows application is in the Software folder. The code is in C#, made on visual studio 2015. In the folder you can see the xaml and xaml.cs files of the project. The app has several pages - main page, control page and pages for the different charts. 
The MainPage is where the connection between the application and the mobile robot must happen. The app is scanning for devices and when the user chooses a concrete device, the app is trying to make the connection via a bluetooth protocol.
The Control page is where the most of the stuff happen. The app sends commands via serial output stream for the direction and speed of the 2 engines of the robot. At the same time reads and collects in Lists the data comming from the robot via serial input stream. After the data comes at the serial port, it is stored in an array and visualized on the ControlPage screen. There is a button that sends the user to the other page - with the concrete charts of a single parameter. The Chart pages are as simple as possible just for an easy visualization of the gathered information and easy processing of this stored data.
The chart pages use the WinRTXamlToolkit.Controls.DataVisualization.Charting, which helps a lot for the making of a chart in C#.

The program that needs to be loaded in the robots microcontoller is in the Firmware folder. The firmware includes 3 .h files and 3 .c files. It's written for PIC18F2550 microchip microcontroller. The main file in this case is PWM2.0.c and the relavent .h file - There the initialization of the controller, the baud speed (which should be changed, depending on the device), the RS232 and I2C communications, the Pulse width modulation is made.
There is a commented part of the code, which can be used if you want to include LEDs in your project for lights, when turning in a concerete direction. If you use them you may want to change the pins, because a feq of them are used for the I2C communication and these cannot be changed.
The sensor - the one used for the measuring of the temperature and humidity is DHT22 and there is a driver written for it. The datasheet of the sensors can be seen on: https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf. The pressure sensor is BMP180. It need a protocol for I2C communication, which is included in the BMP180DRV.c file. the equations used to calculate the temperature and the pressure can be sincronized with: http://wmrx00.sourceforge.net/Arduino/BMP085-Calcs.pdf.

