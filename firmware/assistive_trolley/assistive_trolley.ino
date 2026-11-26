// Reconstructed reference firmware, NOT the recovered original prototype code.
// Target: Arduino Uno R3; HX711 library by bogde.
#include <HX711.h>
#include "sensor_logic.h"
using namespace trolley;
const uint8_t TRIG=2, ECHO=3, IR=4, HX_DT=5, HX_SCK=6, BUZZER=7, RED=8, GREEN=9;
const float LOAD_LIMIT_KG=2.0f; // illustrative software limit, not rated capacity
HX711 scale;
Filter filter;
long zeroRaw=0, latestRaw=0;
float countsPerKg=0, kg=NAN, baseline=NAN, cm=NAN;
bool rawAvailable=false, zeroSet=false, irDetected=false;
uint32_t lastEcho=0,lastLoad=0,lastReport=0;
Proximity nearState=Proximity::Fault;
Load load=Load::Fault;
char command[24]; uint8_t used=0; bool overflow=false;
void executeCommand() {
  command[used]='\0';
  if (!strcmp(command,"t") && rawAvailable && !elapsed(millis(),lastLoad,1000)) {
    zeroRaw=latestRaw; zeroSet=true; filter.reset(); kg=NAN; baseline=NAN;
    Serial.println(F("Tare captured: empty platform required."));
  } else if (!strcmp(command,"c") && zeroSet && rawAvailable && !elapsed(millis(),lastLoad,1000)) {
    // Reference mass must be exactly 1 kg, placed after tare.
    long delta=latestRaw-zeroRaw;
    if (labs(delta)>=100) { countsPerKg=float(delta); filter.reset(); kg=NAN; baseline=NAN; Serial.println(F("1 kg calibration captured.")); }
    else Serial.println(F("Calibration rejected: insufficient signal change."));
  } else Serial.println(F("Commands: t (empty tare), c (1 kg reference); fresh reading required."));
}
void setup() {
  Serial.begin(115200);
  pinMode(TRIG,OUTPUT); digitalWrite(TRIG,LOW); pinMode(ECHO,INPUT);
  pinMode(IR,INPUT_PULLUP); // assumed active LOW reference module
  pinMode(BUZZER,OUTPUT); pinMode(RED,OUTPUT); pinMode(GREEN,OUTPUT);
  digitalWrite(BUZZER,LOW); digitalWrite(RED,HIGH); digitalWrite(GREEN,LOW);
  scale.begin(HX_DT,HX_SCK);
  Serial.println(F("Reconstructed trolley: t=empty tare, c=1 kg calibration. Not a safety device."));
}
void loop() {
  uint32_t now=millis();
  // Bounded input processing; newline required, CR ignored.
  for (uint8_t i=0;i<16 && Serial.available();++i) {
    char ch=Serial.read();
    if(ch=='\n') { if(!overflow) executeCommand(); else Serial.println(F("Command too long")); used=0; overflow=false; }
    else if(ch!='\r') { if(used<sizeof(command)-1 && !overflow) command[used++]=ch; else overflow=true; }
  }
  if(elapsed(now,lastEcho,100)) {
    lastEcho=now; digitalWrite(TRIG,LOW); delayMicroseconds(2); digitalWrite(TRIG,HIGH); delayMicroseconds(10); digitalWrite(TRIG,LOW);
    cm=distanceCm(pulseIn(ECHO,HIGH,25000UL)); // bounded blocking, not fully non-blocking
  }
  irDetected=digitalRead(IR)==LOW;
  nearState=proximity(cm,irDetected);
  if(scale.is_ready()) {
    latestRaw=scale.read(); rawAvailable=true; lastLoad=millis();
    if(zeroSet && countsPerKg!=0) {
      float sample=weightKg(latestRaw,zeroRaw,countsPerKg);
      kg=filter.update(sample);
      if(!isfinite(baseline)) baseline=kg;
      else if(changed(kg,baseline)) { Serial.println(F("Load change detected (not theft confirmation)")); baseline=kg; }
    }
  }
  now=millis();
  if(!rawAvailable || elapsed(now,lastLoad,1000)) { kg=NAN; filter.reset(); baseline=NAN; }
  load=loadState(kg,LOAD_LIMIT_KG);
  bool rangeFault=!isfinite(cm)||cm<2||cm>400;
  bool fault=rangeFault||load==Load::Fault;
  bool critical=nearState==Proximity::Critical;
  bool warning=nearState==Proximity::Warning||load==Load::Overload;
  // Active buzzer module; distinct patterns, obstacle critical takes priority.
  bool beep=critical ? now%200<100 : fault ? now%1000<100 : warning ? now%600<150 : false;
  digitalWrite(BUZZER,beep?HIGH:LOW); digitalWrite(RED,(critical||warning||fault)?HIGH:LOW); digitalWrite(GREEN,(!critical&&!warning&&!fault)?HIGH:LOW);
  if(elapsed(now,lastReport,1000)) {
    lastReport=now;
    Serial.print(F("Distance cm: ")); Serial.print(cm); Serial.print(F(" | IR: ")); Serial.print(irDetected);
    Serial.print(F(" | kg: ")); Serial.print(kg); Serial.print(F(" | Proximity(0 clear,1 warn,2 critical,3 fault): ")); Serial.print(int(nearState));
    Serial.print(F(" | Load(0 normal,1 overload,2 fault): ")); Serial.print(int(load)); Serial.print(F(" | Range fault: ")); Serial.println(rangeFault);
  }
}
