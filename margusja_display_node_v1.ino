/// @dir Ports/lcd_demo
/// Demo sketch for an LCD connected to I2C port via MCP23008 I/O expander.
// 2009-10-31 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
// lcd.backlight();
// lcd.noBacklight();

// include libs
#include <PortsLCD.h>
#include <JeeLib.h>

// define
#define myNodeID 29          //node ID of Rx (range 0-30) 
#define network 210      //network group (can be in the range 1-250).
#define freq RF12_868MHZ     //Freq of RF12B can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. Match freq to module
#define screen_width 16
#define screen_height 2

PortI2C myI2C (4); // P4
LiquidCrystalI2C lcd (myI2C);

typedef struct { int temperature; } PayloadTX; // create structure - a neat way of packaging data for RF comms
PayloadTX emontx;  

const int emonTx_NodeID=10;            //emonTx node ID

void setup() {
  // set up the LCD's number of rows and columns: 
  lcd.begin(screen_width, screen_height);
  // Print a message to the LCD.
  lcd.print("Hello");
  delay(2000);
  
  
  rf12_initialize(myNodeID,freq,network);   //Initialize RFM12 with settings defined above  
  Serial.begin(9600); 
  Serial.println("RF12B demo Receiver - Simple demo"); 
  
   Serial.print("Node: "); 
   Serial.print(myNodeID); 
   Serial.print(" Freq: "); 
   if (freq == RF12_433MHZ) Serial.print("433Mhz");
   if (freq == RF12_868MHZ) Serial.print("868Mhz");
   if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
   Serial.print(" Network: "); 
   Serial.println(network);
}

void loop() {

  lcd.setCursor(0, 0);

  
  
 if (rf12_recvDone()){
  if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0) {
    int node_id = (rf12_hdr & 0x1F);		  //extract nodeID from payload
    lcd.clear();
    lcd.print("Node: ");
    lcd.print(node_id);
     Serial.println(node_id);
        
   //if (node_id == emonTx_NodeID)  {             //check data is coming from node with the corrct ID
       emontx=*(PayloadTX*) rf12_data;            // Extract the data from the payload 
       Serial.print("got temperature: "); Serial.println(emontx.temperature); 
       Serial.println("  "); 
       lcd.setCursor(0,1);
       lcd.print("Temp: ");
       lcd.print(emontx.temperature);
       lcd.print("C");
       
       // sleep some time
       //delay(10000);
  //}
 }
}
}
