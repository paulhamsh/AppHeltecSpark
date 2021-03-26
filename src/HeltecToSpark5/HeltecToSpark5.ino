#include "BluetoothSerial.h"
#include "heltec.h"

#include "Spark.h"
#include "SparkAppIO.h"
#include "SparkIO.h"

#define BUF_MAX 5000

#define PGM_NAME "Heltec - Spark"
#define SCR_HEIGHT 10

#define CONNECT_PIN 19

uint8_t ser_byte, bt_byte;
bool app_connected;

int pos;
int scr_line;
uint8_t pre, eff, drive;

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
int found;
int v[5], vo[5];
int pin[]{32,37,39,38,36};

unsigned int cmdsub;
SparkMessage msg;
SparkPreset preset;
SparkPreset presets[6];

char spark_amps[][STR_LEN]{"RolandJC120", "Twin", "ADClean", "94MatchDCV2", "Bassman", "AC Boost", "Checkmate",
                           "TwoStoneSP50", "Deluxe65", "Plexi", "OverDrivenJM45", // "OverDrivenLuxVerb",
                           "Bogner", "OrangeAD30", //"AmericanHighGain", 
                           "SLO100", "YJM100", "Rectifier",
                           "EVH", "SwitchAxeLead", "Invader", "BE101", "Acoustic", "AcousticAmpV2", //"FatAcousticV2", "FlatAcoustic", 
                           "GK800", "Sunny3000", "W600", "Hammer500"};

char spark_drives[][STR_LEN]{"Booster", "DistortionTS9", //"Overdrive", 
                             "Fuzz", "ProCoRat", "BassBigMuff",
                             "GuitarMuff", // "MaestroBassmaster", 
                             "SABdriver"};

char* effects[7];


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
  eff = 0;
  drive = 0;
  this_delay = 5000;
  app_connected = false;

  for (i = 0; i < 5; i++) {
    v[i]=0;
    vo[i]=0;
  }

  
effects[0] = "bias.noisegate";
effects[1] = "LA2ACompBlueCompCompressorBassCompBBEOpticalComp";
effects[2] = "BoosterDistortionTS9OverdriveFuzzProCoRatBassBigMuffGuitarMuffMaestroBassmasterSABdriver";
effects[3] = "RolandJC120TwinADClean94MatchDCV2BassmanAC BoostCheckmateTwoStoneSP50Deluxe65PlexiOverDrivenJM45OverDrivenLuxVerb"
             "BognerOrangeAD30AmericanHighGainSLO100YJM100RectifierEVHSwitchAxeLeadInvaderBE101AcousticAcousticAmpV2FatAcousticV2"
             "FlatAcousticGK800Sunny3000W600Hammer500";
effects[4] = "TremoloChorusAnalogFlangerPhaserVibrato01UniVibeClonerMiniVibeTremolatorTremoloSquare";
effects[5] = "DelayMonoDelayEchoFiltVintageDelayDelayReverseDelayMultiHeadDelayRe201";
effects[6] = "bias.reverb";
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
    }      
    
    if (cmdsub == 0x0327) {
      selected_preset = msg.param2;
      if (selected_preset == 0x7f) 
        selected_preset=4;
      presets[selected_preset] = presets[5];
      Serial.print("Store in preset: ");
      Serial.println(selected_preset, HEX);
    }

    if (cmdsub == 0x0310) {
      selected_preset = msg.param2;
      if (selected_preset == 0x7f) 
        selected_preset=4;
      presets[5] = presets[selected_preset];
      Serial.print("Hadware preset is: ");
      Serial.println(selected_preset, HEX);
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
      found = -1;
      for (i = 1; found == -1 && i <= 5; i++) {
        if (strstr(effects[i], msg.str1) != NULL) {
          Serial.println (i);
          found = i;
        }
      }
      if (found >= 0) {
        strcpy(presets[5].effects[found].EffectName, msg.str2);
        Serial.print("Change to new effect ");
        Serial.print(msg.str1);
        Serial.print(" ");
        Serial.println(msg.str2);
        dump_preset(presets[5]);        
      }
    }
  }   

  
  
  if (app_connected && ((millis() - count) > this_delay)) {


    if (digitalRead(12) == LOW) {
      this_delay = 400;
      spark_io.change_effect(presets[5].effects[3].EffectName, spark_amps[eff]);
      app_io.change_effect(presets[5].effects[3].EffectName, spark_amps[eff]);
      strcpy(presets[5].effects[3].EffectName, spark_amps[eff]);
      Serial.println(presets[5].effects[3].EffectName);
      eff++;
      if (eff > 25) eff = 1;
    } 
    else if (digitalRead(27) == LOW) {
      this_delay = 400;
      spark_io.change_hardware_preset(pre);
      app_io.change_hardware_preset(pre);
      presets[5] = presets[pre];
      printit("<> Hardware chg");
      pre++;
      if (pre > 3) pre = 0;
    }
    else if (digitalRead(14) == LOW) {
      this_delay = 400;
      spark_io.change_effect(presets[5].effects[2].EffectName, spark_drives[drive]);
      app_io.change_effect(presets[5].effects[2].EffectName, spark_drives[drive]);
      strcpy(presets[5].effects[2].EffectName, spark_drives[drive]);
      Serial.println(presets[5].effects[2].EffectName);
      drive++;
      if (drive > 6) drive = 0;
      
    }
    for (i = 0; i <= 4; i++) {
      v[i]=analogRead(pin[i]) / 64;
      if (abs(v[i] - vo[i]) > 5) {
        this_delay = 50;
        vo[i] = v[i];
        spark_io.change_effect_parameter(presets[5].effects[3].EffectName, i, v[i] / 64.0);
        app_io.change_effect_parameter(presets[5].effects[3].EffectName, i, v[i] / 64.0);
        printit("<> Param chg");           
     
      }
    }

   count = millis();       
  }
}
