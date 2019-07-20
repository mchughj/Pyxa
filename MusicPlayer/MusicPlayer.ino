
//
// Music Player
//

// 7/19/2019
//
// A small program to try out music effects given to me by Kellen.
//
// Required libraries:
//   https://github.com/mchughj/non-blocking-rtttl-arduino
//   https://github.com/adafruit/Adafruit-GFX-Library
//   https://github.com/adafruit/Adafruit-ST7735-Library
// 

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Event.h>
#include <Timer.h>
#include <rtttl-nonblocking.h>

// Pins used in the display 
#define Pyxa_CS     10
#define Pyxa_RST    9
#define Pyxa_DC     8

// Songs used in the game
const char megalovania[] PROGMEM = "Megalovania:d=8,o=5,b=107:16d,16d,d6,a.,g#,g,f,16d,16f,16g,16c,16c,d6,a.,g#,g,f,16d,16f,16g,16b,16b,d6,a.,g#,g,f,16d,16f,16g,16a#,16a#,d6,a.,g#,g,f,16d,16f,16g,16d,16d,d6,a.,g#,g,f,16d,16f,16g,16c,16c,d6,a.,g#,g,f,16d,16f,16g,16b,16b,d6,a.,g#,g,f,16d,16f,16g,16a#,16a#,d6,a.,g#,g,f,16d,16f,16g,16d,16d,d6,a.,g#,g,f,16d,16f,16g,16c,16c,d6,a.,g#,g,f,16d,16f,16g,16b,16b,d6,a.,g#,g,f,16d,16f,16g,16a#,16a#,d6,a.,g#,g,f,16d,16f,16g,16d,16d,d6,a.,g#,g,f,16d,16f,16g,16c,16c,d6,a.,g#,g,f,16d,16f,16g,16b,16b,d6,a.,g#,g,f,16d,16f,16g,16a#,16a#,d6,a.,g#,g,f,16d,16f,16g,f,16f,f,f,f,d,d.,d,16f,16f,16f,f,g,g#,16g,16f,16d,16f,g.,f,16f,f,g,g#,a,c6,a.,d6,d6,16d6,16a,16d6,2c6,16p,a,16a,a,a,a,g,4g,16p,a,16a,a,a,g,a,d6,16a,16g,16p,d6,a,g,f,c6,a,g,f,d,16e,f,a,2c,16p,2p,16f,16d,16f,16g,16g#,16g,16f,16d,16g#,16g,16f,16d,16f,2g,16p,g#,16a,c6,16a,16g#,16g,16f,16d,16e,f,g,a,c6,16p,c#6,g#,16a,16g,16f,g,4p,16p,8p,f4,g4,a4,f,4e,4d,4e,4f,4g,4e,2a,16a,16g#,16g,16f#,16f,16e,16d#,16d,16c#,2d#,2p,16f,16d,16f,16g,16g#,16g,16f,16d,16g#,16g,16f,16d,16e,2g,16p,g#,a,c6,16a,16g#,16g,16f,16d,16e,f,g,a,c6,c#6,g#,16a,16g,16f,g,8p.,8p,f4,g4,a4,f,4e,4d,4e,4f,4g,4e,2a,16a,16g#,16g,16f#,16f,16e,16d#,16d,2c#,2d#,8d#,2a#4.,4f,2e,2d,1f,1f,2a#4.,4f,2e,2d,4p,1d,1d,2a#4.,4f,2e,2d,1f,1f,2a#4.,4f,2e,1d.,1d,2p,16d,16d,d6,a.,g#,g,f,16d,16f,16g,16d,16d,d6,a.,g#,g,f,16d,16f,16g,16c#,16c#,d6,a.,g#,g,f,16d,16f,16g,16c,16c,d6,a.,g#,g,f,16d,16f,16g,16d,16d,d6,a.,g#,g,f,16d,16f,16g,16d,16d,d6,a.,g#,g,f,16d,16f,16g,16c#,16c#,d6,a.,g#,g,f,16d,16f,16g,16c,16c,d6,a.,g#,g,f,16d,16f,16";
const char tetris[] PROGMEM = "Tetris:d=4,o=5,b=150:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";

Adafruit_ST7735 Pyxa = Adafruit_ST7735(Pyxa_CS,  Pyxa_DC, Pyxa_RST);

// Pins of the buttons
#define LEFT_BUTTON A2
#define DOWN_BUTTON A3
#define RIGHT_BUTTON A4 
#define UP_BUTTON A5 
#define Y_BUTTON 7 
#define B_BUTTON 6  
#define A_BUTTON 5  
#define X_BUTTON 2 

// Screen size in pixels
#define SW   128 // Screen width
#define SH   160 // Screen height

ProgmemPlayer soundPlayer(A1);

void setup() {
  Serial.begin(9600);

  pinMode(A1, OUTPUT); // Buzzer is connected to A1 pin

  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(Y_BUTTON, INPUT_PULLUP);
  pinMode(B_BUTTON, INPUT_PULLUP);
  pinMode(A_BUTTON, INPUT_PULLUP);
  pinMode(X_BUTTON, INPUT_PULLUP);

  Pyxa.initR(INITR_BLACKTAB);
  Pyxa.fillScreen(ST7735_BLACK);

  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.setTextSize(1);
  Pyxa.setCursor(30, 20);
  Pyxa.print("Music Player");

  Pyxa.drawRect( 10, 35, SW-20, SH - 45, ST7735_GREEN);
  Pyxa.setTextColor(ST7735_BLUE);
  Pyxa.setCursor(20, 50);
  Pyxa.print("A");
  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.print( " - Megalovania");
  Pyxa.setCursor(20, 70);
  Pyxa.setTextColor(ST7735_RED);
  Pyxa.print("B");
  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.print( " - Tetris");

  Pyxa.setCursor(26, 136);
  Pyxa.setTextColor(ST7735_YELLOW);
  Pyxa.print("X");
  Pyxa.setTextColor(ST7735_GREEN);
  Pyxa.print("Y");
  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.print( " to cancel");
}

bool playing = false;

void loop() {
  if (digitalRead(X_BUTTON) == 0 && digitalRead(Y_BUTTON) == 0 && playing) { 
    playing = false;
    soundPlayer.silence();
    delay(2000);
  }
  if (playing == false && digitalRead(A_BUTTON) == 0) { 
    soundPlayer.setSong(megalovania);  
    playing = true;
  } else if( playing == false && digitalRead(B_BUTTON) == 0) { 
    soundPlayer.setSong(tetris);  
    playing = true;
  }
  if (playing) { 
    if (!soundPlayer.stepSong()) { 
      playing = false;
    }
  }
}

