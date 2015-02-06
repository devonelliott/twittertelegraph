/*****
This is to connect a telegraph sounder to an Arduino and Adafruit's FONA board.
The FONA gets checked for new SMS messages, and when it has some, it translates
the message into Morse code, and sends it to be tapped out on the relay.

I used parts of Adafruit's code example for the FONA, and their BSD license is below.
As of 2015-02-06, the Adafruit FONA library is at https://github.com/adafruit/Adafruit_FONA_Library

I also used Mark Fickett's Arduinomorse library. As of 2015-02-06, it is at https://github.com/markfickett/arduinomorse

Both of those libraries are necessary for this code to work.

*****/

/*************************************************** 
  This is an example for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA 
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to 
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/



#include <SoftwareSerial.h>

#include <morse.h>

#include <Adafruit_FONA.h>

#define TELEGRAPH_PIN	12

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

char replybuffer[255];

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

void setup() {
  
  //Serial.begin(115200);
  
  LEDMorseSender telegraphSender(TELEGRAPH_PIN);
  telegraphSender.setup();
  
  delay (5000); // wait 5 seconds for FONA to connect to network
  
 // make it slow so its easy to read!
  fonaSS.begin(4800); // if you're using software serial
  
  
  // See if the FONA is responding
  if (! fona.begin(fonaSS)) {           // can also try fona.begin(Serial1) 
    //Serial.println(F("Couldn't find FONA"));
    
    telegraphSender.setMessage(String("ooooo"));
    telegraphSender.sendBlocking();
    
    while (1);
  }
  //Serial.println(F("FONA is OK"));
  
  telegraphSender.setMessage(String("sssss"));
  telegraphSender.sendBlocking();
  
}

void loop() {
  
  LEDMorseSender telegraphSender(TELEGRAPH_PIN);
  telegraphSender.setup();
  
  // read the number of SMS's!
        int8_t smsnum = fona.getNumSMS();
        
        if (smsnum < 0) {
          //Serial.println(F("Could not read # SMS"));
          telegraphSender.setMessage(String("ooooo"));
          telegraphSender.sendBlocking();
        } else {
          //Serial.print(smsnum); 
          //Serial.println(F(" SMS's on SIM card!"));
          
          uint16_t smslen;
      
          for (int8_t smsn=1; smsn<=smsnum; smsn++) {
            //Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
            if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
               //Serial.println(F("Failed!"));
               telegraphSender.setMessage(String("ooooo"));
               telegraphSender.sendBlocking();
               break;
            }
        // if the length is zero, its a special case where the index number is higher
        // so increase the max we'll look at!
            if (smslen == 0) {
              //Serial.println(F("[empty slot]"));
              smsnum++;
              continue;
            }
        
            //Serial.print(F("***** SMS #")); Serial.print(smsn); 
            //Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
            //Serial.println(replybuffer);
            //Serial.println(F("*****"));
            String msg = replybuffer;
            msg.toLowerCase();
            String trimmedMsg = msg.substring(0,msg.length()-23); //trim the appended Twitter mobile link from the message
            telegraphSender.setMessage(trimmedMsg);
            telegraphSender.sendBlocking();
            //Serial.println(trimmedMsg);
            fona.deleteSMS(smsn);
            delay(2000);
          }
         }
  
  delay(5000); //wait 5 seconds before polling again
  
}

