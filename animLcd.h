/**
 * @file animLcd.h
 * @brief Header-Datei für den animierbaren lcd (AnimatableLcd)
 * 
 */
#ifndef ANIMLCD_H
#define ANIMLCD_H
#include <LiquidCrystal_I2C.h>
class AnimatableLcd;
#include "animString.h"

const int LOADING_BAR_OFFSET = 2;
/**
 * @brief Eigener Lcd, ermöglicht es Animationen auf dem Lcd Display anzuzeigen
 * 
 */
class AnimatableLcd: public LiquidCrystal_I2C {
    /**
     * @brief Die zurzeit laufende Animation
     * 
     */
    AnimString* animString;
  public:
    /**
     * @brief Gibt an, ob der Monitor animiert werden soll
     * 
     */
    bool doAnimation = false;
    using LiquidCrystal_I2C::LiquidCrystal_I2C; //using the LiquidCrystal constructor
    void setAnimation(AnimString* _animString);
    void printCentered(String text, int length = -1, int row = 0);
    void printPretty(String text);
    void update();
    void init();
    using LiquidCrystal_I2C::print;//übernehme den Standart print befehl
    void print(const String& text); //überschreibt den standart print Befehl für String& (text wird als Variable übergeben, z.B. lcd.print(text);)
    void print(const String&& text);//und String&& (text wird direkt als Argument gegeben, z.B. lcd.print("...");)
};
#endif