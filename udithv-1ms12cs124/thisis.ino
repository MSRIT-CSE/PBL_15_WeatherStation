/*float tempcorrection = 0; //temperature sensor correction(calibration) for
float tempcorrection2 = 0;//temperature sensor correction(calibration)
float humidcorrection = 0; //humidity sensor correction(calibration)
float presscorrection = 0;//pressure sensor correction(calibration)
*/
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <idDHT11.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

int idDHT11pin = 2; //Digital pin for comunications
int idDHT11intNumber = 0; //interrupt number (must be the one that use the previus defined pin (see table above)

//declaration
void dht11_wrapper(); // must be declared before the lib initialization

// Lib instantiate
idDHT11 DHT11(idDHT11pin,idDHT11intNumber,dht11_wrapper);

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
void dht11();
void bmp180();

// Local Network Settings
byte mac[] = { 0x00, 0x26, 0x82, 0x46, 0x11, 0xAB }; // Must be unique on local network
// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "2CCV6J5K5J31NFE8";

String thingtweetAPIKey = "6SN87W76H7O9UFBM";
const int updateThingSpeakInterval = 16 * 1000; // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;
// Initialize Arduino Ethernet Client
EthernetClient client;

//sensor variables
String pres;
String alti;
String humid;
String temp;
void displaySensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}
void setup() {

  Serial.begin(9600);
  Serial.println("Mesuring temperature, humidity and pressure with DHT11 and BMP180");
  Serial.println("");
    Serial.println("Pressure Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displaySensorDetails();

// Start Ethernet on Arduino
startEthernet();
}

void dht11_wrapper() {
  DHT11.isrCallback();
}
void loop() {
if (client.available())
{
char c = client.read();
Serial.print(c);
}
  dht11();
 // delay(500); //we will be waiting 5 seconds till the next mesurement
  //Serial.println("---------------------------------------------------------------------------");
  bmp180();

  // Update ThingSpeak
if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
{
updateThingSpeak("field1="+temp+"&field2="+humid+"&field3="+pres+"&field4="+alti);
}
// Check if Arduino Ethernet needs to be restarted
if (failedCounter > 3 ) {startEthernet();}
lastConnected = client.connected();
  
}


void updateThingSpeak(String tsData)
{
if (client.connect(thingSpeakAddress, 80))
{
client.print("POST /update HTTP/1.1\n");
client.print("Host: api.thingspeak.com\n");
client.print("Connection: close\n");
client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
client.print("Content-Type: application/x-www-form-urlencoded\n");
client.print("Content-Length: ");
client.print(tsData.length());
client.print("\n\n");
client.print(tsData);
lastConnectionTime = millis();
if (client.connected())
{
Serial.println("Connecting to ThingSpeak...");
Serial.println();
failedCounter = 0;
}
else
{
failedCounter++;
Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");
Serial.println();
}
}
}

//startEthernet function
void startEthernet()
{
client.stop();
Serial.println("Connecting Arduino to network...");
Serial.println();
delay(1000);
// Connect to network amd obtain an IP address using DHCP
if (Ethernet.begin(mac) == 0)
{
Serial.println("DHCP Failed, reset Arduino to try again");
Serial.println();
}
else
{
Serial.println("Arduino connected to network using DHCP");
Serial.println();
}
delay(1000);

}
void dht11(){

  
 /* Serial.print("\nRetrieving information from sensor: ");
  Serial.print("Read sensor: ");
  delay(100);*/

  float t=DHT11.getCelsius() ;
  float h=DHT11.getHumidity() ;
  char t_buffer[10];
  char h_buffer[10];
  int result = DHT11.acquireAndWait();
  if(result == IDDHTLIB_OK){
    Serial.println("OK"); 
    }
    else{
      Serial.println("not ok"); 
      }

  Serial.print("Humidity (%): ");
  Serial.println(DHT11.getHumidity(), 2);

  Serial.print("Temperature (oC): ");
  Serial.println(DHT11.getCelsius(), 2);

  Serial.print("Dew Point (oC): ");
  Serial.println(DHT11.getDewPoint());

String temp=dtostrf(t,0,5,t_buffer);
String humid=dtostrf(h,0,5,h_buffer);

  delay(2000); 

}


void bmp180(){
  //pressure variables
            char p_buffer[15];
            char a_buffer[10]; 
             float  p;
             float a;
  /* Get a new sensor event */ 
  sensors_event_t event;
  bmp.getEvent(&event);
 
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    /* Display atmospheric pressue in hPa */
    Serial.print("Pressure:    ");
     p = event.pressure;
    Serial.print(event.pressure);
    Serial.println(" hPa");
    

    float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
    a = bmp.pressureToAltitude(seaLevelPressure,event.pressure);
    Serial.print("Altitude:    "); 
    Serial.print(bmp.pressureToAltitude(seaLevelPressure,
                                        event.pressure)); 
    Serial.println(" m");
    Serial.println("");
  }
  else
  {
    Serial.println("Sensor error");
  }
     pres=dtostrf(p,0,5,p_buffer);
     alti=dtostrf(a,0,5,a_buffer);
  
  delay(1000);
  }
