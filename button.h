/**
   @brief Kleine Klasse die Knopfdrücke verarbeitet
*/
class ButtonHandler { //handels button clicks
    /**
       @brief Der Pin an dem der Knopf angeschlossen ist
    */
    int pin;
    /**
       @brief Gibt an, ob der Knopf momentan Gedrückt ist
    */
    bool isPressed = false;
  public:
    /**
       @brief Die Funktion die bei einem Klick, d.h. einem Drücken und loslassen des Knopfes ausgeführt wird
    */
    void (*onclick)();
    ButtonHandler() {}
    /**
       @brief Erstellt ein neues ButtonHandler Objekt
       @param pin Der Pin an dem der Knopf angeschlossen ist
       @param onclick Die Funktion die bei einem Klick, d.h. einem Drücken und loslassen des Knopfes ausgeführt wird
    */
    ButtonHandler(int pin, void (*onclick)()): pin(pin), onclick(onclick) {
      pinMode(pin, INPUT_PULLUP);
    }
    /**
       @brief Prüft, ob der Knopf gedrückt/losgelassen wurde
       @details Wird von loop() aufgerufen und ruft die ButtonHandler::onclick Funktion auf, wenn ein Klick festgestellt wurde
    */
    void update()
    {
      bool isPressedNew = digitalRead(pin) == HIGH;
      if (isPressedNew != isPressed) { //is not being pressed now, but was being pressed
        if (isPressed) {
          Serial.println("click");
          onclick();
        }
      }
      isPressed = isPressedNew;
    }
};