/****************************************************************
 * $Id: BatteryMonitor.ino 953 2012-12-21 15:02:51Z uk $
 *
 * Battery Monitor for ITfM GmbH
 * using an Arduino + Seeedstudio GSM/GPRS Shield
 *
 * Copyright (c) 2012, Uwe Kamper, All rights reserved
 ****************************************************************/

#include "stdio.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "SoftwareSerial.h"

#include "Voltage.h"
#include "HttpRequest.h"

// variable to store the watchdog flag
volatile int wdt=0;

/****************************************************************
 ** Configuration
 ** Change the variables to your needs.
 ****************************************************************/ 

// APN name for GPRS (e.g. "web.vodafone.de" or "internet.t-d1.de")
#define APN "internet.t-mobile.de"

// username for GPRS (usually empty, "internet" for T-Mobile, Germany/D1)
#define USERNAME "internet" 

// password for GPRS (usually empty, "t-d1" for D1)
#define PASSWORD "t-d1" 

// hostname of the HTTP server where we send our data ...
#define SERVERNAME "teaching.shmooph.com"

// ... and its port number (e.g. 5000 for testing or 80 for production).
#define PORT 80

// a path prefix, must be empty or must start with a slash. 
//Must never end with a slash.
#define PATH_PREFIX "/battery"

// nuber if sleep cycles between each send operation, each cycle is approx. 8 s
#define SLEEP_CYCLES 413

// The name of this host in the icinga configuration
#define HOSTNAME "batt-bla"

/****************************************************************
 ** End of Configuration
 ** Do not change things below this line.
 ****************************************************************/ 


int sensorValue = 0;        // value read from the pot
double voltage = 0.0;
char msg[128] = "/overlord/";

char server_msg[50];
int numdata;
boolean started=false;

int led = 13;

SoftwareSerial mySerial(7, 8);

void printConfig() {
  Serial.print("APN...............");
  Serial.println(APN);
  Serial.print("GPRS username.....");
  Serial.println(USERNAME);
  Serial.print("GPRS password.....");
  Serial.println(PASSWORD);
  Serial.print("Servername........");
  Serial.println(SERVERNAME);
  Serial.print("Port..............");

  // for some reason I have to do it this way or else the program crashes or freezes here
  int port = PORT    ;
  char port_str[5] = "    ";  //reserve the string space first
  itoa(port, port_str, 10); 
  Serial.println(port_str);
  Serial.print("Sleep........");
  // for some reason I have to do it this way or else the program crashes or freezes here
  int sleep = SLEEP_CYCLES;
  char sleep_str[5] = "    ";  //reserve the string space first
  itoa(sleep, sleep_str, 10); 
  Serial.println(sleep_str);
  delay(100);

  Serial.print("Nagios hostname...");
  Serial.println(HOSTNAME);

}

/**
 * Configure the watchdog timer and then present the config menu for 10 .
 */
void setup() {
  // disable the watchdog first in case something went wrong before. This is
  // because watchdog settings stay permanently.
  wdt_disable();

  mySerial.begin(19200);
  
  Serial.begin(19200);
  Serial.println("$Id: BatteryMonitor.ino 953 2012-12-21 15:02:51Z uk $");
  printConfig();
  pinMode(13, OUTPUT);    
}

void loop() {
  
  int i = 0;
  
  // try at least three times
  for (i = 0; i < 3; i++) {
    
    // if the request worked, stop trying.
    if (submitHttpRequest(&mySerial, APN, USERNAME, PASSWORD, SERVERNAME, PORT, msg) == 200) {
      be_evil();
    }
  }

  for(int i=0; i < 120; i++) {
    delay(1000);
  }
  
  // always reboot to avoid with hanging GPRS. I did not look into what the exact 
  // problem is, but GPRS sometimes hangs after many hours of operation. 
  // Rebooting then helps.
  reboot();
}

void be_evil() {
  Serial.println("Robot Overlord is angry");
  long delaytime = 400;
  for(int i=0; i < 200; i++) {
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(delaytime);               // wait for a second
    digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
    delay(delaytime);  
    
    if (delaytime >= 100) {
      delaytime = delaytime - 40;
    }
  }
}






void clearSerial() {
  char buffer[10];
  Serial.readBytesUntil('\n', buffer, 9);
  // Serial.println("<ign");
}

void reboot() {
  wdt_disable();  
  wdt_enable(WDTO_15MS);
  while (1) {}
}

