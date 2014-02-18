//https://github.com/margusja/Atmega328-ds18b20

#include <OneWire.h>
#include <RF12.h>
#include <Ports.h> //from jeelabs.org
//#include <avr/wdt.h>

// 10 - parandatud reguga
// 11 - K88gis - proto1
#define myNodeID 10          //node ID of tx (range 0-30)
#define network     210      //network group (can be in the range 1-250).
#define freq RF12_868MHZ     //Freq of RF12B can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. Match freq to module

typedef struct { int temperature; } PayloadTX;      // create structure - a neat way of packaging data for RF comms
PayloadTX emontx;  

uint32_t timer;

OneWire  ds(7);  // on pin 10

void setup()
{
  
  rf12_initialize(myNodeID,freq,network);   //Initialize RFM12 with settings defined above  
  Serial.begin(9600);
  Serial.println("RFM12B Transmitter - send temperature");

 Serial.print("Node: "); 
 Serial.print(myNodeID); 
 Serial.print(" Freq: "); 
 if (freq == RF12_433MHZ) Serial.print("433Mhz");
 if (freq == RF12_868MHZ) Serial.print("868Mhz");
 if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
 Serial.print(" Network: "); 
 Serial.println(network);
 
 //wdt_enable(WDTO_8S);
}




int do_measurement() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  int celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);         // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present,HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
  
  // convert the data to actual temperature

  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
  }
  //celsius = (float)raw / 16.0;
  celsius = (int)raw / 18;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");

  return celsius;
}

void loop()
{  
  emontx.temperature = do_measurement();
  
  int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
  rf12_sendStart(0, &emontx, sizeof emontx);
    
  Serial.print("sent temperature: "); Serial.println(emontx.temperature); 
  Serial.println("  "); 
  
  delay(250); // waits
  //wdt_reset();
}
