#include <StopWatch.h>
#include <LiquidCrystal.h>

/* Game States:
 0 = Game hasn't started
 1 = game is on and jam isn't on(lineup)
 2 = game is on and jam is on (jam on)
 3 = game isn't on and jam isn't on (timeout)
 4 = game isn't on and jam isn't on and timeout timer has expired
 (official timeout)
 5 = game isn't on and jam isn't on and jam timer is running
 (lineup after timeout)
 6 = game isn't on and jam is on and period timer has expired (LJ)
 7 = game isn't on and jam isn't on and period clock has expired
 (Game over)
 
 */
int prevGameMode;
int gameMode = 0;
int btnJam = 14;
int btnTimeout = 15;
int btnDecreaseTime = 16;
int btnIncreaseTime = 17;
//The elements in this array are used to keep track of button states. Correspond to arduino pins)
int oldBtnState[20];
int state = 0;
int vibeNum = 3;
int jamNum = 0;
int periodNum = 1;
unsigned long btnDelay = 0;
unsigned long btnTimer = 0;
unsigned long lcdDelay = 100;
unsigned long lcdTimer = 0;
unsigned long periodLength = 1800000;
String displayString;
// The elements of this array correspond to what the 'gameTimer' clock should be counting.
// 30 secs for lineup, 60 secs for a timeout, and 120 secs for a jam.
// use gameMode as the index when reading elements.
unsigned long segmentLength[] = {
  30000 ,30000, 120000, 60000, 0, 30000, 120000, 0};
String gameModeString[] = {
  "Pre-game ", "Line up  ", "Jam On   ", "Time Out ", "Official ", "Line Up ", "Last Jam ", "Game Over"};
LiquidCrystal lcd( 7, 8, 9, 10, 11, 12);
// Actual pins on TQFP 32 (32, 1, 2, 9, 10, 11, 12)
StopWatch periodTimer;
StopWatch gameTimer;

boolean checkButton(int pin, int mode = -1); // Overload for the checkButton function

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
   lcd.print("Jam Boss");
   lcd.setCursor(9,1);
   lcd.print("by icon");
   delay(1500);
   lcd.clear(); 
}

void loop() {

  handleEvents();

  // Now that we know which mode the game is in, we can update the clocks
  // and display as needed.

  updateClocks();

  // And since the clocks have been updated we should display the information.

  updateDisplay();

} // void loop()


//                                                                                 ///////////////// void handleEvents()
void handleEvents() {

  // This function handles events like timer expiration and button presses.
  // Changes the game mode accordingly.

  if (checkButton(btnDecreaseTime)) {
    periodLength -= 25;
  }

  if (checkButton(btnIncreaseTime)) {
    periodLength += 25;
  }

  switch (gameMode) {
  case 0: // Game hasn't started.
    // Wait for Jam button press
    checkButton(btnJam, 2);

  case 1: // Lineup
    // Check for button input
    // on Jam button press go to 2
    checkButton(btnJam, 2);
    // on Timeout press go to 3
    checkButton(btnTimeout, 3);
    // on timer expired go to 2
    checkGameTimer(2);
    checkPeriodTimer(0);
    break;
  case 2: // Jam On
    checkButton(btnJam, 1);
    checkButton(btnTimeout, 3);
    checkGameTimer(1);
    checkPeriodTimer(6);
    break;
  case 3: // Timeout
    checkButton(btnJam, 2);
    checkButton(btnTimeout, 4);
    checkGameTimer(5);
    break;
  case 4: // Official TO
    checkButton(btnJam, 2);
    checkButton(btnTimeout, 5);
    // No need to check clocks here. We are in limbo.
    break;
  case 5: // Lineup after timeout
    checkButton(btnJam, 2);
    checkButton(btnTimeout, 4);
    checkGameTimer(3);
    // We don't need to check clocks here because the game is paused.
    break;
  case 6: // Last Jam
    checkButton(btnJam, 7);
    checkButton(btnTimeout, 4);
    checkGameTimer(7);
    // checkPeriodTimer(0);
    break;
  case 7: // Game Over
    checkButton(btnJam, 0);
    break;
  } // Switch()
}

void updateClocks() {
  //This function manipulates the timers when the game mode changes.
  if (gameMode != prevGameMode) {
    prevGameMode = gameMode;
    switch (gameMode) {
      // When the game mode changes to 'Not Started'
    case 0:
      periodTimer.reset();
      gameTimer.reset();
      periodNum++;
      jamNum = 0;
      break;
      // Lineup
    case 1:
      if (!periodTimer.isRunning()) {
        periodTimer.start();
      }
      endSegment();
      gameTimer.start();
      break;
      // Jam on
    case 2:
      if (!periodTimer.isRunning()) {
        periodTimer.start();
      }
      endSegment();
      gameTimer.start();
      jamNum++;
      break;
      // Time out
    case 3:
      periodTimer.stop();
      endSegment();
      gameTimer.start();
      break;
      // Official Timeout
    case 4:
      endSegment();
      break;
      // Line up after time out (period clock not running)
    case 5:
      endSegment();
      gameTimer.start();
      break;
      // Last jam
    case 6:
      // Do nothing here. The jam clock has already been started and the period clock
      // has stopped.
      break;

      // Game over
    case 7:
      gameTimer.stop();
      periodTimer.stop();
      break;
    } // switch

  } // if

} // function

//                                                                                 ///////////////// void updateDisplay()

void updateDisplay() {
  if (!millis() - lcdTimer > lcdDelay) {
    return;
  }

  displayTimer(gameTimer.elapsed(), segmentLength[gameMode], -1, 0);
  displayGameMode();
  displayTimer(periodTimer.elapsed(), periodLength, 9, 0);
  displayCounts();
}

void checkPeriodTimer(int mode) {
  if (periodTimer.elapsed() > periodLength) {
    gameMode = mode;
  }
}

//                                                                                 ///////////////// void displayClock()
void displayTimer(unsigned long elapsed, unsigned long totalTime, int x, int y) {
  unsigned long minutes, seconds, decimal, timeLeft;
  displayString = "";

  timeLeft = totalTime - elapsed;
  seconds = constrain(timeLeft / 1000, 0, totalTime / 1000);
  decimal = constrain((timeLeft - seconds * 1000) / 100,0,9);
  minutes = seconds / 60;
  seconds %= 60;

  if ((elapsed < totalTime)) {
    if (minutes < 10) {
      displayString += " ";
    }

    displayString += String(minutes);
    displayString += ":";

    if (seconds < 10) {
      displayString += "0";
    }

    displayString += String(seconds);
    displayString += ".";
    displayString += String(decimal);
  }
  else {
    displayString = " 0:00.0";
  }

  lcd.setCursor(x,y);
  lcd.print(displayString);

} // function

void displayGameMode() {
  lcd.setCursor(0,1);
  lcd.print(gameModeString[gameMode]);
}

//                                                                                 ///////////////// void endSegment()
void endSegment() {
  // Just a quick function to stop the game clock.
  gameTimer.stop();
  gameTimer.reset();
}

void checkGameTimer(int changeMode) {
  if (gameTimer.elapsed() > segmentLength[gameMode]) {
    gameMode = changeMode;
  } 
}

//                                                                                 /////////////// void checkButton(int pin, int mode


boolean checkButton(int pin, int mode) {
  // Checks to see if a button on the specified pin was pressed.
  state = digitalRead(pin);
  if (state == HIGH) {
    if (oldBtnState[pin] == LOW) {
      if ((millis() - btnTimer) >= btnDelay) {
        btnTimer = millis();
        if (mode != -1) {
          gameMode = mode;
        } // if
      } // if
      btnTimer = millis();
    } // if

  }
  oldBtnState[pin] = state;
} // function

void displayCounts() {
  lcd.setCursor(10, 1);
  lcd.print("J");
  lcd.print(jamNum);
  lcd.print(" P");
  lcd.print(periodNum); 
}














