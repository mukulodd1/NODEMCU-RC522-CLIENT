/*
 * ----------------------------------------------------------------------
 * Example program showing how to read new NUID from a PICC to serial.
 * ----------------------------------------------------------------------
 * https://circuits4you.com
 * 
 * RC522 Interfacing with NodeMCU
 * 
 * Typical pin layout used:
 * ----------------------------------
 *             MFRC522      Node     
 *             Reader/PCD   MCU      
 * Signal      Pin          Pin      
 * ----------------------------------
 * RST/Reset   RST          D1 (GPIO5)        
 * SPI SS      SDA(SS)      D2 (GPIO4)       
 * SPI MOSI    MOSI         D7 (GPIO13)
 * SPI MISO    MISO         D6 (GPIO12)
 * SPI SCK     SCK          D5 (GPIO14)
 * 3.3V        3.3V         3.3V
 * GND         GND          GND
 */

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
const char* ssid = "Realme 2 Pro";
const char* password = "realme2pro123";
const char* host = "192.168.43.113";


constexpr uint8_t RST_PIN = 5;     // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 4;     // Configurable, see typical pin layout above
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

void setup() { 
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println();

  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);

  while(WiFi.status()!= WL_CONNECTED)
  {
    delay(500);
    Serial.println(".");
   }
  Serial.println("WiFi connected");
  Serial.print("IP Address =");
  Serial.println(WiFi.localIP());
  
}
 
void loop() {

  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
    String str="";
    for(int i=0;i<rfid.uid.size;i++)
    str = str+String(rfid.uid.uidByte[i]);
    Serial.println(str);
    Serial.print("Connecting to");
  Serial.println(host);
  WiFiClient client;
  const int  httpPort =33;
  if(!client.connect(host,httpPort))
  {
    Serial.println("Connection Failed");
    return ;
  }
   String url="/query?language="+str;
   
   Serial.print("requesting URL: ");
   Serial.println(url);
   client.print(String("GET ")+url+ "\r\n"+
   "Host: " + host +"\r\n"+
   "Connection  : close\r\n\r\n");
   unsigned long timeout = millis();
   
   while(client.available()==0)
   {
    if(millis()-timeout>5000)
    {
      Serial.println(">>>Client Timeout");
      client.stop();
      return ;
    }
   }
   while(client.available())
   {
     String line = client.readStringUntil('\r');
     Serial.println(line);
   }
   client.stop();
   delay(2000);
    
    /*Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();*/
  }
  else Serial.println(F("Card read previously."));
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
