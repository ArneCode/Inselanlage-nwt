#include "animLcd.h"
class Pager;
class PageContent;

#include "page.h"
#include "pager.h"
//#include "button.h"

const byte ladePin = 13;
const byte notstromPin = 12;
const byte ventilatorPin = 11;
const byte beleuchtungsPin = 10;
const byte rPin = 9; //B
const byte gPin = 6;
const byte bPin = 5;
const byte ventilatorTasterPin = 8;
const byte beleuchtungsTasterPin = 7;
const byte lautsprecherPin = 4;
const byte rAkkustandPin=3;
const byte bAkkustandPin=2;
const byte SOLAR_RE_V_PIN = A0;
const byte SOLAR_GES_V_PIN = A1;
const byte CAP_V_PIN = A2;
const byte gAkkustandPin=A3;

const int ALERT_DURATION = 2000;

const int beleuchtungsSpannung[2] = {
  300,
  810
}; //Schwellspannung für Beleuchtung, ideal mit n*255 Differenz

const char* ventilatorStufen[3] = {
  "aus",
  "langsam",
  "schnell",
};
const char* beleuchtungsStufen[4] = {
  "aus",
  "dunkel",
  "hell",
  "automatisch"
};

AnimatableLcd lcd(0x27, 16, 2);
Pager pager;
Option<LcdString*> alert = Option<LcdString*>::None();
bool is_alert = false;
void display_alert(LcdString* animation, bool block_multiple = false) {
  //auto animation = new LcdDotAnim("Willkommen zur Inselanlage", &lcd);
  static unsigned long last_alert = -1;
  if (millis() - last_alert < 5000 && block_multiple) {
    delete animation;
    return;
  }
  last_alert = millis();
  end_alert();
  alert = Option<LcdString*>::Some(animation);
  animation->run();
  is_alert = true;
}
void end_alert() {
  if (alert.is_set) {
    delete alert.get_value();
    alert = Option<LcdString*>::None();
  }
}

float read_volts(int pin) {
  return (analogRead(pin) * 5.0 / 1024.0);
}
int uSolarRe;
int uSolarGes = 0;
int uKond = 0;

bool ventilatorTasterGedrueckt = false;
bool betaetigtTasterVentilator = false;
bool beleuchtungsTasterGedrueckt = false;
bool betaetigtTasterBeleuchtung = false;

//vom Nutzer kontrollierte Einstellungen
byte ventilatorStufe = 0;
byte beleuchtungsStufe = 0;
byte feierBeleuchtungsStufe = 0;
byte rgbVerblassDurchlauf = 1;
bool sollLaden = true;
bool sollNotstrom = false;
bool istReihenschaltung = true;

String mV = "mV";

unsigned int rgbVerlaufsZeitRest;
unsigned int ersterRgbVerlaufsZeitRest;
void setup() {
  // put your setup code here, to run once:
  pinMode(ladePin, OUTPUT);
  pinMode(notstromPin, OUTPUT);
  pinMode(ventilatorPin, OUTPUT);
  pinMode(beleuchtungsPin, OUTPUT);
  pinMode(rPin, OUTPUT);
  pinMode(ventilatorTasterPin, INPUT);
  pinMode(beleuchtungsTasterPin, INPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
  pinMode(lautsprecherPin, OUTPUT);
  Serial.begin(9600);
  lcd.init();
  Serial.setTimeout(0);
  const int n_pages = 4;
  auto pages = new Page*[n_pages] {
    new DynamicContentPage("Solar L/R: ", []() {
      return String(uSolarGes - uSolarRe) + mV + " " + String(uSolarRe) + mV;
    }),
    new DynamicContentPage("Sol Ges/Cap:", []() {
      return String(uSolarGes) + mV+" " + String(uKond) + mV;
      }),
      /*
    new DynamicContentPage("Solar Ges:", []() {
      return String(uSolarGes) + "mV";
    }),
    new DynamicContentPage("Kondensator: ", []() {
      return String(uKond) + "mV";
    }),*/
    new DynamicContentPage("Ventilator: ", []() {
      //return String("Stufe ") + String(ventilatorStufe);
      return String(ventilatorStufen[ventilatorStufe]);
    }),
    new DynamicContentPage("Beleuchtung: ", []() {
      //return String("Stufe ") + String(beleuchtungsStufe);
      return String(beleuchtungsStufen[beleuchtungsStufe]);
    }),
   /* new DynamicContentPage("Schaltung: ", []() {
      if (istReihenschaltung) {
        return String("Reihe");
      } else {
        return String("Parallel");
      }
    }),*/
  };
  pager = Pager(pages, n_pages);
  //pager.display(&lcd);
  //alert = Option<AnimString>::Some("Hallo");
  display_alert(new LcdLoadingAnim("Lade", &lcd, 3000));
}



void loop() {
  // put your main code here, to run repeatedly:
  /*static unsigned long nextMesureT = 0;
    bool t1000 = false;
    if (millis() > nextMesureT){
    nextMesureT = millis() + 1000;
    t1000 = true;
    }*/
  static unsigned int volleSekunden = -1; //-1 wird zum größmöglichen Wert, bewirkt, dass t1000 beim ersten Durchlauf wahr ist
  //Einmal pro Sekunde wahr
  bool t1000 = millis() / 1000 != volleSekunden; //Wenn z.B. millis()/1000 1900 ergibt, wird mit 1000 weitergearbeitet, da es eine int-Variable ist
  if (t1000) volleSekunden = millis() / 1000;
  //Spannungsmessung
  bool wirdGeladen = sollLaden &&
                     uSolarGes >= uKond &&
                     uKond <= 3000 &&
                     !sollNotstrom;
  if (t1000 && (wirdGeladen || uKond < 1250)) {
    digitalWrite(ladePin, 0);
    noTone(lautsprecherPin);
    delay(10); //Zur richtigen Spannungsmessung werden Solarzellen kurz von Kondensatoren getrennt, sonst wird Spannung zu hoch gemessen
  }
  if (t1000 || !wirdGeladen) { //Wenn mit Kondensatoren verbunden wird Spannung nur sekündlich gemessen
    uSolarRe = map(analogRead(SOLAR_RE_V_PIN), 0, 1023, 0, 5000);
    uSolarGes = map(analogRead(SOLAR_GES_V_PIN), 0, 1023, 0, 5000);
    uKond = map(analogRead(CAP_V_PIN), 0, 1023, 0, 5000);
  }
  //
  if (t1000 && wirdGeladen) digitalWrite(ladePin, 1);

  //Ausgabe serieller Monitor sekündlich
  if (t1000) {
    Serial.print("Solarzellen:");
    Serial.print(sollLaden);
    Serial.print(" Notstrom:");
    Serial.print(sollNotstrom);
    Serial.print(" Ventilator:");
    Serial.print(ventilatorStufen[ventilatorStufe]);
    Serial.print(" Feierbeleuchtung:");
    Serial.print(feierBeleuchtungsStufe);
    Serial.print(" Beleuchtung:");
    //if (beleuchtungsStufe == 3) Serial.println("automatisch");
    //else Serial.println(beleuchtungsStufe);
    Serial.print(beleuchtungsStufen[beleuchtungsStufe]);
    Serial.print(" ");
    if (istReihenschaltung) {
      Serial.println("Reihenschaltung");
      Serial.print("Linke Solarzelle in mV:   ");
      Serial.println(uSolarGes - uSolarRe);
      Serial.print("Rechte Solarzelle in mV:  ");
      Serial.println(uSolarRe);
    } else Serial.println("Parallelschaltung");
    Serial.print("Solarzellen gesamt in mV: ");
    Serial.println(uSolarGes);
    Serial.print("Kondensator in mV:        ");
    Serial.println(uKond);
  }

  //Verarbeitung Tastereingabe
  if (digitalRead(ventilatorTasterPin)) {
    if (!ventilatorTasterGedrueckt) {
      ventilatorTasterGedrueckt = true;
      betaetigtTasterVentilator = true;
    } else betaetigtTasterVentilator = false;
  } else {
    ventilatorTasterGedrueckt = false;
    betaetigtTasterVentilator = false;
  }
  if (digitalRead(beleuchtungsTasterPin)) {
    if (!beleuchtungsTasterGedrueckt) {
      beleuchtungsTasterGedrueckt = true;
      betaetigtTasterBeleuchtung = true;
    } else betaetigtTasterBeleuchtung = false;
  } else {
    beleuchtungsTasterGedrueckt = false;
    betaetigtTasterBeleuchtung = false;
  }

  //Verarbeitung Tastatureingabe
  char taste = Serial.read();
  if (taste == 'v' || betaetigtTasterVentilator) {
    ventilatorStufe ++;
    if (ventilatorStufe > 2){
      ventilatorStufe = 0;
    }
    Serial.print("Ventilator ");
    /*if (ventilatorStufe == 0) {
      ventilatorStufe++;
      Serial.println("langsam");
    } else if (ventilatorStufe == 1) {
      ventilatorStufe++;
      Serial.println("schnell");
    } else {
      ventilatorStufe = 0;
      Serial.println("aus");
    }*/
    Serial.println(ventilatorStufen[ventilatorStufe]);
  }
  if (taste == 'b' || betaetigtTasterBeleuchtung) {
    Serial.print("Beleuchtung ");
    beleuchtungsStufe++;
    if(beleuchtungsStufe>3){
      beleuchtungsStufe = 0;
    }
    Serial.println(beleuchtungsStufen[beleuchtungsStufe]);
    /*if (beleuchtungsStufe == 0) {
      beleuchtungsStufe++;
      Serial.println("dunkel");
    } else if (beleuchtungsStufe == 1) {
      beleuchtungsStufe++;
      Serial.println("hell");
    } else if (beleuchtungsStufe == 2) {
      beleuchtungsStufe++;
      Serial.println("automatisch");
    } else {
      beleuchtungsStufe = 0;
      Serial.println("aus");
    }*/

  }
  if (taste == 's') {
    istReihenschaltung = !istReihenschaltung;
    Serial.print("Auf ");
    if (istReihenschaltung) {
      Serial.print("Reihen");
      display_alert(new LcdString("Reihen", &lcd, ALERT_DURATION));
    }
    else {
      Serial.print("Parallel");
      display_alert(new LcdString("Parallel", &lcd, ALERT_DURATION));
    }
    Serial.println("schaltung umgestellt");
  } else if (taste == 'l') {
    sollLaden = !sollLaden;
    Serial.print("Laden ");
    if (!sollLaden) Serial.print("de");
    Serial.println("aktiviert");
  }
  else if (taste == 'f') {
    feierBeleuchtungsStufe++;
    Serial.print("Feierbeleuchtung ");
    if (feierBeleuchtungsStufe == 1)Serial.println("langsam");
    else if (feierBeleuchtungsStufe == 2)Serial.println("schnell");
    else if (feierBeleuchtungsStufe == 3)Serial.println("sehr schnell");
    else if (feierBeleuchtungsStufe == 4)Serial.println("langsam verblassend");
    else if (feierBeleuchtungsStufe == 5)Serial.println("schnell verblassend");
    else {
      feierBeleuchtungsStufe = 0;
      Serial.println("aus");
    }
  }
  else if (taste == 'n') {
    sollNotstrom = !sollNotstrom;
    Serial.print("Notstrom ");
    if (!sollNotstrom) {
      Serial.print("de");
      display_alert(new LcdString("Notstrom aus", &lcd, ALERT_DURATION), true);
      }else{
        display_alert(new LcdString("Notstrom an", &lcd, ALERT_DURATION), true);
      }
    //else Serial.println("deaktiviert");
    Serial.println("aktiviert");
    //display_alert(new LcdString("Notstrom an", &lcd, 1000), true);
  }
  else if(taste=='m')verbrenner();

  //Steuerung Laden von Kondensatoren
  if (sollNotstrom) {
    if (uKond > 4500 || t1000) {
      Serial.print("Notstrom ");
    }
    if (uKond > 4500) {
      sollNotstrom = false;
      Serial.println("deaktiviert, Kondensator mehr als 4000mV");
      //display_alert(new LcdString("Notstrom deaktiviert", &lcd, 1000));
    } else {
      digitalWrite(ladePin, 0);
      digitalWrite(notstromPin, 1);
      if (t1000) {
        //Serial.println("Kondensator wird mit Notstrom geladen");
        Serial.println("an");
      }
    }
  } else if (sollLaden && uKond <= uSolarGes && uKond <= 3000) {
    digitalWrite(notstromPin, 0);
    digitalWrite(ladePin, 1);
    if (t1000) Serial.println("Speicher wird geladen");
  } else {
    digitalWrite(notstromPin, 0);
    digitalWrite(ladePin, 0);
    if (t1000) {
      Serial.print("Speicher wird nicht geladen");
      if (uKond > 3000) Serial.println(", da mehr als 3000mV");
      else Serial.println("");
    }
  }

  //Steuerung Verbraucher
  analogWrite(ventilatorPin, 127.5 * ventilatorStufe);
  if (beleuchtungsStufe == 3) {
    if (uSolarGes <= beleuchtungsSpannung[0]) digitalWrite(beleuchtungsPin, 1);
    else if (uSolarGes < beleuchtungsSpannung[1])
      analogWrite(beleuchtungsPin, map(uSolarGes, beleuchtungsSpannung[0], beleuchtungsSpannung[1], 255, 0));
    else digitalWrite(beleuchtungsPin, 0);
  } else analogWrite(beleuchtungsPin, 127.5 * beleuchtungsStufe);
  if (uKond < 500) {
    tone(lautsprecherPin, millis() % 1000 + 200);
    display_alert(new LcdString("!ALARM, SPEICHER LEER!", &lcd, ALERT_DURATION), true);
  }
  else if (uKond < 750 && millis() / 1000 % 2 == 0) {
    tone(lautsprecherPin, millis() % 1000 + 200);
    display_alert(new LcdString("ALARM, SPEICHER "+uKond+String("mV") , &lcd, ALERT_DURATION), true);
  }
  else if (uKond < 1000 && millis() / 1000 % 2 == 0) {
    tone(lautsprecherPin, 400);
    display_alert(new LcdString("Alarm, Speicher fast leer", &lcd, ALERT_DURATION), true);
  }
  else if (uKond < 1250 && millis() / 1000 % 4 == 0) {
    tone(lautsprecherPin, 400);
    display_alert(new LcdString("Speicher fast leer", &lcd, ALERT_DURATION), true);
  }
  else noTone(lautsprecherPin);
  if (feierBeleuchtungsStufe > 3)rgbVerblassend(1000 / pow(feierBeleuchtungsStufe - 3, 2));
  else if (feierBeleuchtungsStufe > 0)rgb(1000 / pow(feierBeleuchtungsStufe, 3));
  else {
    digitalWrite(rPin, 0);
    digitalWrite(gPin, 0);
    digitalWrite(bPin, 0);
  }
  if (alert.is_set) {
    // while(Serial.available() == 0){
    lcd.update();
    //  }
    if (Serial.available() != 0 || alert.get_value()->isDone()) {
      Serial.readString();
      //alert = Option<LcdString*>::None();
      end_alert();
      pager.display(&lcd);
    }
  } else {
    pager.update(&lcd);
    //if (Serial.available() != 0) {
    if (taste == ' ') {
      //String result = Serial.readString();
      /*Serial.print("input: '");
        Serial.print(result);
        Serial.println("'");*/
      pager.next_page();
      pager.display(&lcd);
    }
  }
  /*if(uKond<500){
    digitalWrite(gAkkustandPin,0);
    digitalWrite(rAkkustandPin,1);
  }
  else */if(uKond<1250){
    digitalWrite(gAkkustandPin,0);
    //analogWrite(rAkkustandPin,pow(2,map(uKond,500,1249,80,1)/10.0));
    digitalWrite(rAkkustandPin,1);//analogWrite(rAkkustandPin,map(uKond,500,1249,255,1));
  }
  else if(uKond<2000){
    digitalWrite(gAkkustandPin,1);//analogWrite(gAkkustandPin,map(uKond,1250,2000,0,255));
    analogWrite(rAkkustandPin,pow(2,map(uKond,1250,2000,80,0)/10.0));
    //digitalWrite(rAkkustandPin,1);
  }
  else{
    digitalWrite(rAkkustandPin,0);
    digitalWrite(gAkkustandPin,1);
  }
  if(sollNotstrom)digitalWrite(bAkkustandPin,millis()%200<100);
  else if(wirdGeladen)digitalWrite(bAkkustandPin,1);
  else digitalWrite(bAkkustandPin,0);
}

void rgb(int rgbVerlaufsZeit) {
  if (millis() / rgbVerlaufsZeit % 3 == 0) {
    digitalWrite(bPin, 0);
    digitalWrite(rPin, 1);
  }
  else if (millis() / rgbVerlaufsZeit % 3 == 1) {
    digitalWrite(rPin, 0);
    digitalWrite(gPin, 1);
  }
  else {
    digitalWrite(gPin, 0);
    digitalWrite(bPin, 1);
  }
}
void rgbVerblassend(int rgbVerlaufsZeit) {
  rgbVerlaufsZeitRest = millis() % rgbVerlaufsZeit;
  if (millis() / rgbVerlaufsZeit % 6 == 0) {
    digitalWrite(gPin, 0);
    digitalWrite(bPin, 1);
    if (rgbVerblassDurchlauf == 1)ersterRgbVerlaufsZeitRest = rgbVerlaufsZeitRest;
    analogWrite(rPin, map(rgbVerlaufsZeitRest, ersterRgbVerlaufsZeitRest, rgbVerlaufsZeit - 1, 0, 255));
    rgbVerblassDurchlauf = 2;
  }
  else if (millis() / rgbVerlaufsZeit % 6 == 1) {
    digitalWrite(gPin, 0);
    digitalWrite(rPin, 1);
    if (rgbVerblassDurchlauf == 2)ersterRgbVerlaufsZeitRest = rgbVerlaufsZeitRest;
    analogWrite(bPin, map(rgbVerlaufsZeitRest, ersterRgbVerlaufsZeitRest, rgbVerlaufsZeit - 1, 255, 0));
    rgbVerblassDurchlauf = 3;
  }
  else if (millis() / rgbVerlaufsZeit % 6 == 2) {
    digitalWrite(bPin, 0);
    digitalWrite(rPin, 1);
    if (rgbVerblassDurchlauf == 3)ersterRgbVerlaufsZeitRest = rgbVerlaufsZeitRest;
    analogWrite(gPin, map(rgbVerlaufsZeitRest, ersterRgbVerlaufsZeitRest, rgbVerlaufsZeit - 1, 0, 255));
    rgbVerblassDurchlauf = 4;
  }
  else if (millis() / rgbVerlaufsZeit % 6 == 3) {
    digitalWrite(bPin, 0);
    digitalWrite(gPin, 1);
    if (rgbVerblassDurchlauf == 4)ersterRgbVerlaufsZeitRest = rgbVerlaufsZeitRest;
    analogWrite(rPin, map(rgbVerlaufsZeitRest, ersterRgbVerlaufsZeitRest, rgbVerlaufsZeit - 1, 255, 0));
    rgbVerblassDurchlauf = 5;
  }
  else if (millis() / rgbVerlaufsZeit % 6 == 4) {
    digitalWrite(rPin, 0);
    digitalWrite(gPin, 1);
    if (rgbVerblassDurchlauf == 5)ersterRgbVerlaufsZeitRest = rgbVerlaufsZeitRest;
    analogWrite(bPin, map(rgbVerlaufsZeitRest, ersterRgbVerlaufsZeitRest, rgbVerlaufsZeit - 1, 0, 255));
    rgbVerblassDurchlauf = 6;
  }
  else {
    digitalWrite(rPin, 0);
    digitalWrite(bPin, 1);
    if (rgbVerblassDurchlauf == 6)ersterRgbVerlaufsZeitRest = rgbVerlaufsZeitRest;
    analogWrite(gPin, map(rgbVerlaufsZeitRest, ersterRgbVerlaufsZeitRest, rgbVerlaufsZeit - 1, 255, 0));
    rgbVerblassDurchlauf = 1;
  }
}
void verbrenner() {
  delay(2000);
  digitalWrite(ventilatorPin, 1);
  delay(500);
  analogWrite(ventilatorPin, 100);
  delay(3000);
  digitalWrite(ventilatorPin, 1);
  delay(1200);
  for (int i = 150; i < 256; i++) {
    analogWrite(ventilatorPin, i);
    delay(15);
  }
  delay(500);
  for (int i = 170; i < 256; i++) {
    analogWrite(ventilatorPin, i);
    delay(25);
  }
  delay(400);
  for (int i = 190; i < 256; i++) {
    analogWrite(ventilatorPin, i);
    delay(50);
  }
  delay(200);
  for (int i = 200; i < 256; i++) {
    analogWrite(ventilatorPin, i);
    delay(100);
  }
  delay(1000);
  for (int i = 255; i > 200; i--) {
    analogWrite(ventilatorPin, i);
    delay(50);
  }
  digitalWrite(ventilatorPin, 1);
  delay(500);
  for (int i = 255; i > 200; i--) {
    analogWrite(ventilatorPin, i);
    delay(40);
  }
  for (int i = 255; i > 150; i--) {
    analogWrite(ventilatorPin, i);
    delay(30);
  }
  for (int i = 255; i > 100; i--) {
    analogWrite(ventilatorPin, i);
    delay(10);
  }
  delay(3000);
  digitalWrite(ventilatorPin, 0);
}
