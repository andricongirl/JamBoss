#include <StopWatch.h>
#include <Adafruit_CharacterOLED.h>

// To keep track of the Jam or Lineup
StopWatch timerJam;
// To keep track of the Period
StopWatch timerPeriod;
// To keep track of the timeout
StopWatch timerTimeOut;
// Initialize the OLED display
Adafruit_CharacterOLED lcd(6, 7, 8, 9, 10, 11, 12);


unsigned long jamLength = 11000;
unsigned long lineupLength = 30000;
unsigned long periodLength = 10000;
unsigned long timeoutLength = 60000;
unsigned long refresh;                                    // Keeps track of the display refresh
unsigned long jamRef = 100;                               // Sets the jam clock refresh rate
unsigned long periodRef = 100;                            // Period clock refresh rate
unsigned long debounceTimer;
unsigned long debounceAmount = 250;
boolean GameOn, JamOn, TimeOut;                           // Status of the game; current phase.
int JamNum = 1;                                           // What jam are we on?
int jamButton = 2;
int timeoutButton = 3;

//=================================================================
// void setup()
//=================================================================

void setup() {
  lcd.begin(16, 2);
}

//=================================================================
// void loop()
//=================================================================

void loop() {
  int jamButtonState = digitalRead(jamButton);
  int timeoutButtonState = digitalRead(timeoutButton);

  // Check the buttons for input
  if (jamButtonState == HIGH && (millis() - debounceTimer) > debounceAmount) {
    if (TimeOut) {
      timerTimeOut.stop();
      timerTimeOut.reset();
      TimeOut = 0;
    }
    toggleJam();
    debounceTimer = millis();
  }
  if (timeoutButtonState == HIGH && (millis() - debounceTimer) > debounceAmount) {
    if (!TimeOut) {
      startTimeOut();
    }
    debounceTimer = millis();
  }

  // Has the period clock expired?
  if ((timerPeriod.elapsed() > periodLength) && GameOn) {      
    if (timerPeriod.isRunning()) {
      timerPeriod.stop();
    }
    if((timerJam.elapsed() > jamLength) && JamOn) {
      GameOn = 0;
      JamOn = 0;
      timerPeriod.reset();
      JamNum = 1;
    }
  }
  // Has the jam clock expired?
  if ((timerJam.elapsed() > jamLength) && JamOn && GameOn) {
    if (timerPeriod.isRunning() && GameOn) {
    toggleJam();
    }
    else {
      timerJam.stop();
      timerJam.reset();
      JamOn = 0;
      GameOn = 0;
    }
  }
  // Has the lineup clock expired?
  if ((timerJam.elapsed() > lineupLength) && !JamOn && GameOn) {
    toggleJam();
  }
  // Has the timeout clock expired?
  if ((timerTimeOut.elapsed() > timeoutLength) && TimeOut && GameOn) {      
    timerTimeOut.stop();
  }


  // Check to make sure we are only refreshing every (displayRes) ms.
  if ((millis() - refresh) > jamRef) {                  
    displayJamNum();                                           
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
    GameOn = 1;
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
  String displayString;
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
    displayString += " ";
  }
  //                                                        // Print a colon if there are minutes...
  if (minutes) {
    displayString += String(minutes);
    displayString += ":";
  }
  //                                                        // ... or pad it out if there aren't minutes
  else {
    displayString += "  ";
  }
  //                                                        // Pad seconds out with zeros.
  if (secs < 10) {
    displayString += "0";
  }
  //                                                        // Print secs and milliseconds with a dot.
  displayString += String(secs);
  displayString += ".";
  displayString += String(decimal);

  lcd.setCursor(9, 0);
  lcd.print(displayString);
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

  lcd.print(constrain(JamNum, 1, 99));

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
  String displayString;

  secs = constrain (secs %= 60, 0, 59);
  decimal = constrain((togo - secs*1000) / 100, 0, 9);

  if (minutes < 10) {
    displayString += " ";
  }

  if (minutes) {
    displayString += String(minutes);
    displayString += ":";
  }
  // ... or pad it out if there aren't minutes
  else {
    displayString += "  ";
  }
  // Pad seconds out with zeros.
  if (secs < 10) {
    displayString += "0";
  }
  // Print secs and milliseconds with a dot.
  displayString += String(secs);
  displayString += ".";
  displayString += String(decimal);

  lcd.setCursor(9, 0);
  lcd.print(displayString);
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
  String displayString;

  if (minutes < 10) {
    displayString += " ";
  }
  displayString += String(minutes);
  displayString += ":";

  if (secs < 10) {
    displayString += "0";

  }
  displayString += String(secs);

  if (TimeOut) {
    displayString += "   Time Out";
  }
  if (JamOn) {
    displayString += "     Jam On";    
  }
  if (!TimeOut && !JamOn) {
    displayString += "    Line Up";  
  }

  lcd.setCursor(0,1);
  lcd.print(displayString);

}












