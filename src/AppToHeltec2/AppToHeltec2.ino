#include "BluetoothSerial.h" //Header File for Serial Bluetooth, will be added by default into Arduino
#include "heltec.h"

BluetoothSerial SerialBT; //Object for Bluetooth
HardwareSerial HWSerial(2);

#define HW_BAUD 1000000
#define PGM_NAME "App - Heltec"
#define CONNECT_PIN 27
#define BUF_MAX 5000

uint8_t bt_byte, ser_byte, b;
uint8_t rx_buf[BUF_MAX];
int rx_pos, rx_len;
int tx_pos, tx_len, tx_cmd, tx_sub;
int scr_line;

void printit(char *str) {
  if (scr_line >= 8) {
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, PGM_NAME);
    scr_line = 1;
  }

  Heltec.display->drawString(0,scr_line *8, str);
  Heltec.display->display();
  scr_line++;
}

#define SPARK_BT_NAME "Spark 40 Audio"

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);
  HWSerial.begin(HW_BAUD, SERIAL_8N1, 14, 12);

  pinMode(CONNECT_PIN, INPUT);

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, PGM_NAME);
  Heltec.display->display();
  scr_line = 1;

  printit("Waiting for other Heltec");
  delay(1000);
  while (digitalRead(CONNECT_PIN) == LOW);
  printit("Other Heltec is ready");

  printit("Starting..");
  if (!SerialBT.begin(SPARK_BT_NAME)) 
  {
    printit("Failed"); 
    while (true) {};
  }
  printit("Ready ");
  
  while (HWSerial.available()) 
    b = HWSerial.read(); 
  while (SerialBT.available())
    b = SerialBT.read();
    
  rx_pos = 0;
  rx_len = -1;
  tx_pos = 0;
  tx_len = -1;
}

char str[40];

void loop() {
  
  while (SerialBT.available()) {
    bt_byte = SerialBT.read();
    tx_pos++;
    
    if (tx_pos == 7) 
      tx_len = bt_byte;
    if (tx_pos == 21) 
      tx_cmd = bt_byte;
    if (tx_pos == 22) 
      tx_sub = bt_byte;
 
    if (tx_pos == tx_len) {
      tx_pos = 0;
      tx_len = -1; 
      sprintf(str, "> %2.2x%2.2x", tx_cmd, tx_sub);
      printit(str);
    }

    HWSerial.write(bt_byte);    
  };

  
  while (HWSerial.available()) {
    ser_byte = HWSerial.read();

    if (rx_pos > BUF_MAX) {
      Serial.println("BUST MY BUFFER");
      while (true);
    }
    
/*    
    if (ser_byte < 16)
      Serial.print(" 0");
    else
      Serial.print(" ");      
    Serial.print(ser_byte,HEX);
*/    
    rx_buf[rx_pos++] = ser_byte;
    if (rx_pos == 7) {
      rx_len = ser_byte; 
    }
    
    if (rx_pos == rx_len) {
      SerialBT.write(rx_buf, rx_pos);  

//      Serial.println(" <<<<<");
      rx_pos = 0;
      rx_len = -1;  
      
      sprintf(str, "< %2.2x%2.2x", rx_buf[20], rx_buf[21]);
      printit(str);
    }
  }
}
