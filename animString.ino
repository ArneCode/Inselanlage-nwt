/**
 * @file animString.ino
 * @brief Implementationen der Callable und LcdString Klassen
 * 
 */
#include "animLcd.h"
//#include "callHandler.h"
#include "header.h"
#include "animString.h"
/**
 * @brief ruft die angegebene Funktion auf
 * 
 */
void FuncCall::run() 
{
  call();
}
/**
 * @brief Gibt zurück, ob der nächste Call ausgeführt werden sollte
 * 
 * @return true 
 * @return false 
 */
bool FuncCall::isDone() 
{
  return _isDone();
}
/**
 * @brief gibt den String (LcdString::text) auf dem Lcd-Display aus
 * 
 */
void LcdString::run() 
{
  callStart = millis();
  lcd->doAnimation = false;
  lcd->printPretty(this->text);
}
/**
 * @brief Gibt zurück, ob die duration überschritten ist
 * 
 * @return true 
 * @return false 
 */
bool LcdString::isDone() 
{
  return millis() - callStart > duration;
}
/**
 * @brief Setzt Variablen die für alle Animationen notwendig sind und ruft dann die eigene init() Funktionen auf, die von Abgeleiteten Klassen definiert wird
 * 
 */
void AnimString::run() 
{
  callStart = millis();
  lcd->clear();
  lcd->setAnimation(this);
  animStart = millis();
  lastRefresh = millis();
  init();
}
/**
 * @brief Initialisierung der Ladeanimation
 */
void  LcdLoadingAnim::init() 
{
  stepDuration = duration / 9;
  if (text.length() > 16) {
    Serial.print("warning: text given for loading animation is to long, text: ");
    Serial.println(text);
  }
  lcd->printCentered(text);
  lcd->setCursor(LOADING_BAR_OFFSET, 1);
  for (int i = 0; i < 8; i++) {
    lcd->write(0);
  }
  lcd->print("0% ");
}
/**
 * @brief Aktualisiert die Ladeanimation, wird von loop() aufgerufen
 * 
 */
void LcdLoadingAnim::update() 
{
  time_t time = millis();
  short percent = (time - animStart) * 100 / duration;
  if (time - lastRefresh > stepDuration) {
    short nToFill = percent * 9 / 100;
    if (nToFill == 0) {
      return;
    }
    lcd->setCursor(nToFill + LOADING_BAR_OFFSET - 1, 1);
    lcd->write(1);
    lastRefresh = time;
  }
  lcd->setCursor(8 + LOADING_BAR_OFFSET, 1);
  lcd->print(percent);
  lcd->print("%");
}
/**
 * @brief Initialisiert die Punktanimation
 * 
 */
void LcdDotAnim::init() 
{
  lcd->printPretty(text + "\1\1\1");//spaces that can't be broken up to newlines
}
/**
 * @brief Aktualisiert die Punktanimation, wird von loop() aufgerufen
 * 
 */
void LcdDotAnim::update() 
{
  time_t time = millis();
  if ((time - lastRefresh) < stepDuration) {
    return;
  }
  lastRefresh = time;
  int numDots = ((time - animStart) / stepDuration) % 4;
  char dots[4];
  for (int i = 0; i < 3; i++) {
    if (i < numDots) {
      dots[i] = '.';
    } else {
      dots[i] = '\1';
    }
  }
  dots[3] = '\0';
  lcd->printPretty(text + dots);
}