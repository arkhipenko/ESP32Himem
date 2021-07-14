#include "ESP32Himem.h"


ESP32Himem himem;

void setup() {
  Serial.begin(115200);

  int rc = himem.begin();
 
  
  Serial.print("Himem begin rc = ");
  Serial.println(rc);
  
  if (rc == 0) {
    Serial.print("Himem physical size: ");
    Serial.println(himem.physSize());
    Serial.print("Himem allocated size: ");
    Serial.println(himem.size());
    Serial.print("Himem max buffer size: ");
    Serial.println(himem.bufferSize());
    Serial.print("Himem pointer: ");
    Serial.println((uint32_t)himem.pointer(), HEX);
    
    size_t s = himem.size();
    size_t st = 0;
    
    Serial.println ("\nPopulating Himem area... "); 
    Serial.print("Start time: ");
    Serial.println(millis());
    himem.seek(st);
    for (size_t i = st; i < st+s; i++) {
      if ( himem.write( i % 256 ) == 0 ) {
        Serial.print("ERROR writing to HIMEM at address ");
        Serial.println(i);
        while(1){}
      }
    }
    Serial.print("Stop  time: ");
    Serial.println(millis());
    
    Serial.println ("\nReading Himem area via Read "); 
    Serial.print("Start time: ");
    Serial.println(millis());
    himem.seek(st);
    for (size_t i = st; i < st+s; i++) {
      int a = himem.read();
      if ( a != (i % 256) ) {
        Serial.println("ERROR reading data back");
        Serial.print("Read: ");
        Serial.println(a);
        Serial.print("Expected: ");
        Serial.println((i%256));
        Serial.print("At address: ");
        Serial.println(i);
        while(1){}
      }
    }
    Serial.print("Stop  time: ");
    Serial.println(millis());
 
    Serial.println ("\nReading Himem area via [] "); 
    Serial.print("Start time: ");
    Serial.println(millis());
    himem.seek(st);
    for (size_t i = st; i < st+s; i++) {
      int a = himem[i];
      if ( a != (i % 256) ) {
        Serial.println("ERROR reading data back");
        Serial.print("Read: ");
        Serial.println(a);
        Serial.print("Expected: ");
        Serial.println((i%256));
        Serial.print("At address: ");
        Serial.println(i);    
        while(1){}
      }
    }
    Serial.print("Stop  time: ");
    Serial.println(millis()); 


    Serial.println ("\nReading Himem area randomly 1000 times "); 
    Serial.print("Start time: ");
    Serial.println(millis());
    himem.seek(st);
    for (size_t i = 0; i < 1000; i++) {
      size_t j = random(0, s);
      int a = himem[j];
      if ( a != (j % 256) ) {
        Serial.println("ERROR reading data back");
        Serial.print("Read: ");
        Serial.println(a);
        Serial.print("Expected: ");
        Serial.println((j%256));
        Serial.print("At address: ");
        Serial.println(j);    
        while(1){}
      }
    }
    Serial.print("Stop  time: ");
    Serial.println(millis());     
    
    
    Serial.println ("\nWriting and reading Himem area buffer directly "); 
    Serial.print("Start time: ");
    Serial.println(millis());
    himem.seek(st);
    uint8_t* p = himem.pointer();
    size_t j = 0;
    
    for (size_t i = 0; i < himem.bufferSize() - himem.bufferIndex(); i++) {
      p[j] = i%256;
      uint8_t a = p[j];
      if ( a != (i % 256) ) {
        Serial.println("ERROR reading data back via pointer");
        Serial.print("Read: ");
        Serial.println(a);
        Serial.print("Expected: ");
        Serial.println((j%256));
        Serial.print("At address: ");
        Serial.println(j);    
        while(1){}
      }
    }
    Serial.print("Stop  time: ");
    Serial.println(millis());    
  }
}



void loop() {
  
}
