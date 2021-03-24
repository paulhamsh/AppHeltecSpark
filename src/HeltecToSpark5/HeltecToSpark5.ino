#include "BluetoothSerial.h"
#include "heltec.h"

#include "Spark.h"
#include "SparkAppIO.h"
#include "SparkIO.h"

#define BUF_MAX 5000

#define PGM_NAME "Heltec - Spark"
#define SCR_HEIGHT 10

#define CONNECT_PIN 23

uint8_t ser_byte, bt_byte;
bool app_connected;

int pos;
int scr_line;
uint8_t pre, eff;

uint8_t buf[BUF_MAX];

unsigned long this_delay;
unsigned long count;

SparkAppIO app_io;
SparkIO spark_io;
SparkComms spark_comms;

char str[STR_LEN];
uint8_t selected_preset;

uint8_t b;
int i, j, p;
int v1,v2;

unsigned int cmdsub;
SparkMessage msg;
SparkPreset preset;
SparkPreset presets[6];

char spark_amps[][STR_LEN]{"RolandJC120", "Twin", "ADClean", "94MatchDCV2", "Bassman", "AC Boost", "Checkmate",
                           "TwoStoneSP50", "Deluxe65", "Plexi", "OverDrivenJM45", "OverDrivenLuxVerb",
                           "Bogner", "OrangeAD30","AmericanHighGain", "SLO100", "YJM100", "Rectifier",
                           "EVH", "SwitchAxeLead", "Invader", "BE101", "Acoustic", "AcousticAmpV2", "FatAcousticV2",
                           "FlatAcoustic", "GK800", "Sunny3000", "W600", "Hammer500"};


byte get_serial[]{0x01, 0xfe, 0x00, 0x00, 0x53, 0xfe, 0x17, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                  0xf0, 0x01, 0x01, 0x01, 0x02, 0x23, 0xf7};

byte set_preset[]{0x01, 0xfe, 0x00, 0x00, 0x53, 0xfe, 0x1a, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                  0xf0, 0x01, 0x01, 0x01, 0x01, 0x38, 
                  0x00, 0x00, 0x01, 0xf7}; 
                  
byte upd_preset[]{0x01, 0xfe, 0x00, 0x00, 0x41, 0xff, 0x1a, 0x00,
                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                  0xf0, 0x01, 0x01, 0x01, 0x03, 0x38, 
                  0x00, 0x00, 0x01, 0xf7}; 


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

void dump_preset(SparkPreset preset) {
  int i,j;

  Serial.print(preset.curr_preset); Serial.print(" ");
  Serial.print(preset.preset_num); Serial.print(" ");
  Serial.print(preset.Name); Serial.print(" ");

  Serial.println(preset.Description);

  for (j=0; j<7; j++) {
    Serial.print("    ");
    Serial.print(preset.effects[j].EffectName); Serial.print(" ");
    if (preset.effects[j].OnOff == true) Serial.print(" On "); else Serial.print (" Off ");
    for (i = 0; i < preset.effects[j].NumParameters; i++) {
      Serial.print(preset.effects[j].Parameters[i]); Serial.print(" ");
    }
    Serial.println();
  }
}
void flush_in() {
  bt_byte = spark_io.comms->bt->read();
  while (bt_byte != 0xf7)
    bt_byte = spark_io.comms->bt->read();
}

void send_preset(uint8_t pres) {
//  set_preset[24] = pres;
//  spark_io.bt.write(set_preset, 26);
//  flush_in(); 

  spark_io.change_hardware_preset(pres);
  spark_io.process();

  
//  upd_preset[24] = pres;
  // this is actually the 8 bit xor checksum, 
  // but for this short message it is just the same as the preset number.  
//  upd_preset[19] = pres;  
// this was the fix 
//  upd_preset[19] = 0x01; 

//  upd_preset[18] = seq++;
//  if (pres == 0) upd_preset[19] = 0x01; // this works
//  if (pres == 1) upd_preset[19] = 0x02; // this works
//  if (pres == 2) upd_preset[19] = 0x01; // this works
//  if (pres == 3) upd_preset[19] = 0x01; // 0x02 works, 0x01, 0x04 works - just not 0x03
//  app_io.bt->write(upd_preset, 26);
  app_io.change_hardware_preset(pres);
  app_io.process();
}

void setup() {

//  HWSerial.begin(HW_BAUD, SERIAL_8N1, 33, 32);

  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);

  pinMode(CONNECT_PIN, OUTPUT);
  digitalWrite(CONNECT_PIN, LOW);

  pinMode(12,INPUT_PULLUP);  
  pinMode(14,INPUT_PULLUP);  
  pinMode(27,INPUT_PULLUP);  
  pinMode(26,INPUT_PULLUP); 
  
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, PGM_NAME);
  Heltec.display->display();
  scr_line = 1;

  spark_io.comms = &spark_comms;
  app_io.comms = &spark_comms;

  spark_comms.start_bt();
  spark_comms.connect_to_spark();
  
  spark_comms.start_ser();
  digitalWrite(CONNECT_PIN, HIGH);
  printit("Connected");
  pos = 0;
  count = millis();
  pre = 0;
  eff = 1;
  this_delay = 5000;
  app_connected = false;

  v1 = 0;
  v2 = 0;
}


  
void loop() {

  spark_io.process();
  app_io.process();
  
  if (spark_io.get_message(&cmdsub, &msg, &preset)) { //there is something there
    count = millis();
    
    Serial.print("From Spark: ");
    Serial.println(cmdsub, HEX);
    sprintf(str, "< %4.4x", cmdsub);
    printit(str);
    
    if (cmdsub == 0x0301) {
      p = preset.preset_num;
      j = preset.curr_preset;
      if (p == 0x7f)       
        p = 4;
      if (j == 0x01)
        p = 5;
      presets[p] = preset;
    }

    if (cmdsub == 0x0306) {
      strcpy(presets[5].effects[3].EffectName, msg.str2);
      Serial.print("Change to amp model ");
      Serial.println(presets[5].effects[3].EffectName);
    }
    
    if (cmdsub == 0x0338) {
      selected_preset = msg.param2;
      presets[5] = presets[selected_preset];
      Serial.print("Change to preset: ");
      Serial.println(selected_preset, HEX);
//      dump_preset(presets[5]);
    }      
    
    if (cmdsub == 0x0327) {
      selected_preset = msg.param2;
      if (selected_preset == 0x7f) 
        selected_preset=4;
      presets[selected_preset] = presets[5];
      Serial.print("Store in preset: ");
      Serial.println(selected_preset, HEX);
//      dump_preset(presets[5]);
    }
  }

   if (app_io.get_message(&cmdsub, &msg, &preset)) { //there is something there
    count = millis();

    Serial.print("To Spark: ");
    Serial.println(cmdsub, HEX);
    sprintf(str, "> %4.4x", cmdsub);
    printit(str);

    if (cmdsub == 0x022f) app_connected = true;
    
    if (cmdsub == 0x0101) {
      p = preset.preset_num;
      if (p == 0x7f) 
        p = 4;
      presets[p]=preset;  
      Serial.print("Send new preset: ");
      Serial.println(p, HEX);      
    }

    if (cmdsub == 0x0138) {
      selected_preset = msg.param2;
      if (selected_preset == 0x7f) 
        selected_preset=0;
      presets[5] = presets[selected_preset];
      Serial.print("Change to preset: ");
      Serial.println(selected_preset, HEX);
//      dump_preset(presets[5]);
    }

    if (cmdsub == 0x0106) {
      strcpy(presets[5].effects[3].EffectName, msg.str2);
      Serial.print("Change to new effect ");
      Serial.print(msg.str1);
      Serial.print(" ");
      Serial.println(msg.str2);
      dump_preset(presets[5]);
    }
   
  }   

  
  
  if (app_connected && ((millis() - count) > this_delay)) {
    this_delay = 200;

    if (digitalRead(12) == LOW) {
      spark_io.change_effect(presets[5].effects[3].EffectName, spark_amps[eff]);
      app_io.change_effect(presets[5].effects[3].EffectName, spark_amps[eff]);
      strcpy(presets[5].effects[3].EffectName, spark_amps[eff]);
      Serial.println(presets[5].effects[3].EffectName);
      eff++;
      if (eff > 8) eff = 1;
    } 
    else if (digitalRead(14) == LOW) {
      spark_io.change_hardware_preset(pre);
      app_io.change_hardware_preset(pre);
      presets[5] = presets[pre];
      printit("<> Hardware chg");
      pre++;
      if (pre > 3) pre = 0;
    }

    v1 = analogRead(32) / 256;   // only have 16 values
    if (v1 != v2) {
      v2 = v1;
      spark_io.change_effect_parameter(presets[5].effects[3].EffectName, 0, v1 / 16.0);
      app_io.change_effect_parameter(presets[5].effects[3].EffectName, 0, v1 / 16.0);
      printit("<> Param chg");           
    }
   count = millis();       
  }
}
