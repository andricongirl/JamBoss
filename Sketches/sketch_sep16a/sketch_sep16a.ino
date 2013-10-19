#include <StopWatch.h>
#include <Adafruit_CharacterOLED.h>

StopWatch timerJam;
StopWatch timerPeriod;
StopWatch timerTimeOut;
Adafruit_CharacterOLED lcd(6, 7, 8, 9, 10, 11, 12);

unsigned long jamLength = 120000;
unsigned long lineupLength = 30000;
unsigned long periodLength = 10000;
unsigned long timeoutLength = 60000;
unsigned long refresh;                                    // Keeps track of the display refresh
unsigned long jamRef = 100;                               // Sets the jam clock refresh rate
unsigned long jamBlink = 300;                             // Speed that the LED blinks
unsigned long fadeSpeed = 40;                             // Speed that the LED Fades
unsigned long blinkTimer = 0;                            // Variable to time the LED.
unsigned long debounceTimer = 0;
unsigned long debounceAmount = 50;
boolean JamOn, TimeOut;                                   // Status of the game; current phase.
int JamNum = 1;                                           // What jam are we on?
int fadeAmount = 5;                                       // How much does the LED Fade?
int ledBrightness = 10;                                    // Current brightness of the LED
int ledPin = 5;
int jamButton = 2;
int timeoutButton = 3;

//=================================================================
// void setup()
//=================================================================

void setup() {

  pinMode(ledPin, OUTPUT);

  lcd.begin(16, 2);
  Serial.begin(115200);
  Serial.flush();
}

//=================================================================
// void loop()
//=================================================================

void loop() {
  int jamButtonState = digitalRead(jamButton);
  int timeoutButtonState = digitalRead(timeoutButton);

  if (jamButtonState == HIGH && (millis() - debounceTimer) > 250) {
    if (TimeOut) {
      timerTimeOut.stop();
      timerTimeOut.reset();
      TimeOut = 0;
    }
    toggleJam();
    debounceTimer = millis();
  }
  if (timeoutButtonState == HIGH && (millis() - debounceTimer) > 250) {
    if (!TimeOut) {
      startTimeOut();
    }
    debounceTimer = millis();
  }


  if ((timerJam.elapsed() > jamLength) && JamOn) {          // Has the jam clock expired?
    toggleJam();
  }

  if ((timerJam.elapsed() > lineupLength) && !JamOn) {      // Has the lineup clock expired?
    toggleJam();
  }

  if ((timerTimeOut.elapsed() > timeoutLength) && TimeOut) {      // Has the timeout clock expired?
    timerTimeOut.stop();
    //timerTimeOut.reset();
  }
  //                                                        // This is a placeholder for an actual button
  if (Serial.available()) {                                 // Is there serial data available?
    switch (Serial.read()) {                                // Read serial data
    case '1':                                               // If it's a 1 then press the Jam Button
      //                                                    // Check to see if we're coming back from Timeout
      if (TimeOut) {
        timerTimeOut.stop();
        timerTimeOut.reset();
        TimeOut = 0;
      }
      toggleJam();
      break;

      //                                                      // If it is a 2 then press the timeout button 
    case '2':
      if (!TimeOut && timerJam.isRunning()) {
        if (JamOn) {
          JamNum++;
        }
        startTimeOut();
      }
      break;
    }

    Serial.flush();                                         // Clear the buffer so we can start fresh
  }

  if (JamOn) {
    // Turn the LED off during the jam
    digitalWrite(ledPin, LOW);
  }

  if (!JamOn && !TimeOut) {
    // Blink the LED During lineup
    if (timerJam.elapsed() < 20000) {
      jamBlink = 300;
    }
    else {
      jamBlink = 100; 
    }
    if ((millis() - blinkTimer) > jamBlink) {
      digitalWrite(ledPin, !digitalRead(ledPin));
      blinkTimer = millis();
    }

  }

  if (TimeOut) {
    if ((millis() - blinkTimer) > fadeSpeed) {
      analogWrite(ledPin, ledBrightness);
      ledBrightness = ledBrightness + fadeAmount;
      if (ledBrightness <= 10 || ledBrightness >= 220) {
        fadeAmount = -fadeAmount ; 
      }   
      blinkTimer = millis();
    }

  }


  if ((millis() - refresh) > jamRef) {                  // Check to make sure we are only refreshing
    displayJamNum();                                           // every (displayRes) ms.
    if (!TimeOut) {
      displayJamClock();
    }
    else {
      displayTimeOut();
    }
    displayPeriod();
    refresh = millis();
  }
}

//=================================================================
// void startTimeOut()    Stops the clocks and starts a timeout
//=================================================================

void startTimeOut() {

  JamOn = 0;
  TimeOut = 1;
  timerJam.reset();
  timerPeriod.stop();
  timerTimeOut.start();

}

//=================================================================
// void toggleJam()    Self Explanatory.
//=================================================================

void toggleJam() {                                          // Check to see if the jam is on or not
  //                                                        // if not, stop the lineup timer and begin
  if(!timerPeriod.isRunning()) {                            // the jam timer
    timerPeriod.start();
  }

  switch (JamOn) {                                          
  case 0:
    timerJam.stop();
    timerJam.reset();
    timerJam.start();
    break;

  case 1:
    timerJam.stop();
    timerJam.reset();
    timerJam.start();
    JamNum++;                                                // Increment the jam after the jam is over.
    break;
  }
  JamOn = !JamOn;
}

//=================================================================
// displayJamClock()     Formats the clocks output for display
//=================================================================

void displayJamClock() {                                    
  //                                                        // elapsed = milliseconds on the timer
  unsigned long elapsed = timerJam.elapsed();               // minutes = whole number extracted from elapsed
  unsigned long secs, decimal, minutes, togo;               // decimal = number after the decimal point

  switch (JamOn) {

  case 1:                                                   // Format the jam time remaining.
    togo = jamLength - elapsed;
    secs = constrain(togo / 1000, 0, jamLength);
    decimal = constrain((togo - secs*1000) / 100, 0, 9);
    break;
    // Format the lineup time remaining.
  case 0:
    togo = lineupLength - elapsed;
    secs = constrain(togo / 1000, 0, lineupLength);
    decimal = constrain((togo - secs*1000) / 100, 0, 9);
    break;
  }
  lcd.setCursor(0, 1);
  lcd.print(togo / 1000);
  // Convert seconds to minutes and secs
  if (secs >= 60) {
    minutes = secs / 60;
    secs %= 60;
  }
  else {
    minutes = 0;
  }
  //                                                        // Pad minutes out with blank space
  if (minutes < 10) {
    Serial.print(" ");
  }
  //                                                        // Print a colon if there are minutes...
  if (minutes) {
    Serial.print(minutes);
    Serial.print(":");
  }
  //                                                        // ... or pad it out if there aren't minutes
  else {
    Serial.print("  ");
  }
  //                                                        // Pad seconds out with zeros.
  if (secs < 10) {
    Serial.print("0");
  }
  //                                                        // Print secs and milliseconds with a dot.
  Serial.print(secs);
  Serial.print(".");
  Serial.print(decimal);
  //                                                        // Next line.
  Serial.println("");
}

//=================================================================
// displayJamNum()     Print the current jam number with padding
//=================================================================

void displayJamNum() { 
  lcd.setCursor(0,0);
  lcd.print("Jam ");
  if (JamNum < 10) {
    lcd.print(" ");
  }

  lcd.print(JamNum);
  lcd.print("   ");

}


//=================================================================
// displayTimeOut()     Print the Timeout clock
//=================================================================

void displayTimeOut() { 
  unsigned long elapsed = constrain(timerTimeOut.elapsed(), 0, timeoutLength);
  unsigned long togo = constrain(timeoutLength - elapsed, 0, timeoutLength);
  unsigned long secs = togo / 1000;
  unsigned long minutes = constrain(secs / 60, 0, 59);
  unsigned long decimal;

  secs = constrain (secs %= 60, 0, 59);
  decimal = constrain((togo - secs*1000) / 100, 0, 9);

  if (minutes < 10) {
    Serial.print(" ");
  }

  if (minutes) {
    Serial.print(minutes);
    Serial.print(":");
  }
  // ... or pad it out if there aren't minutes
  else {
    Serial.print("  ");
  }
  // Pad seconds out with zeros.
  if (secs < 10) {
    Serial.print("0");
  }
  // Print secs and milliseconds with a dot.
  Serial.print(secs);
  Serial.print(".");
  Serial.print(decimal);
  // Next line.
  Serial.println("");


}

//=================================================================
// displayPeriod()     Print the Period clock
//=================================================================

void displayPeriod() {
  unsigned long elapsed = constrain(timerPeriod.elapsed(), 0, periodLength);
  unsigned long togo = constrain(periodLength - elapsed, 0, periodLength);
  unsigned long secs = togo / 1000;
  unsigned long minutes = constrain(secs / 60, 0, 59);
  secs = constrain (secs %= 60, 0, 59);

  if (minutes < 10) {
    Serial.print(" ");
  }
  Serial.print(minutes);
  Serial.print(":");

  if (secs < 10) {
    Serial.print("0");

  }
  Serial.print(secs);

  if (TimeOut) {
    Serial.println("   Time Out");
  }
  if (JamOn) {
    Serial.println("     Jam On");    
  }
  if (!TimeOut && !JamOn) {
    Serial.println("    Line Up");  
  }

}









