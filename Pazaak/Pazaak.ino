
//
// Pazaak
//

//
// Required libraries:
//   https://github.com/ponty/arduino-rtttl-player
//   https://github.com/adafruit/Adafruit-GFX-Library
//   https://github.com/adafruit/Adafruit-ST7735-Library
// 

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <Event.h>
#include <Timer.h>
#include <rtttl.h>

// Pins used in the display 
#define Pyxa_CS     10
#define Pyxa_RST    9
#define Pyxa_DC     8

// Songs used in the game
const char gameBeginSound[] PROGMEM = "B:d=4,o=5,b=400:8b,2e..6";
const char clearLineSound[] PROGMEM = "ClearLine:d=8,o=6,b=500:c,e,d7,c,e,a#,c,e,a";
const char hitBottomSound[] PROGMEM = "SmallBeep:d=8,o=6,b=500:4d";

Adafruit_ST7735 Pyxa = Adafruit_ST7735(Pyxa_CS,  Pyxa_DC, Pyxa_RST);

// Semantic datatype name for byte
#define byte unsigned char

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

// Screen layout
//   +--------------+
//   |    Players   |
//   |     Hand     |   40 Pixels
//   +--------------+
//   |    Current   |   60 Pixels
//   |     Round    |
//   +--------------+
//   |    Current   |   20 Pixels
//   |     score    |
//   +--------------+   
//   | Instructions |   40 Pixels
//   +--------------+

// Player's hand:
//   Need to show 4 cards
//   Assume need buffer on the top and bottom of 3 pixels each.
//   Assume that space is dilineated between areas by a line.
//   7 pixels out so 33 pixels.
//   Make a card 26 pixels by 26 pixels.  4 * 26 = 104 pixels wide.
//   Assume that there is 2 pixels on both sides of each card.  
//
//   |  +--------------------------+    +--------------------------+   +--------------------------+   +------ ...
//   |  |                          |    |                          |   |                          |   |       ...
//   |  |                          |    |                          |   |                          |   |       ...
//   |  |            X             |    |                          |   |      X                   |   |       ...
//   |  |            X             |    |                          |   |      X                   |   |       ...
//   |  |            X             |    |                          |   |      X          X        |   |       ...
//   |  |       xxxxxxxxxxx        |    |        xxxxxxxxxxx       |   |   xxxxxxx      X         |   |       ...
//   |  |            X             |    |                          |   |      X        X          |   |       ...
//   |  |            X             |    |                          |   |      X       X           |   |       ...
//   |  |            X             |    |                          |   |      X      X    xxxxxx  |   |       ...
//   |  |                          |    |                          |   |            X             |   |       ...
//   |  |                          |    |                          |   |           X              |   |       ...
//   |  |                          |    |                          |   |                          |   |       ...
//   |  |          xxx             |    |         xxxxxx           |   |           xxxxx          |   |       ...
//   |  |         x  X             |    |        x      x          |   |          x     x         |   |       ...
//   |  |        x   X             |    |                x         |   |                 x        |   |       ...
//   |  |            X             |    |                x         |   |                x         |   |       ...
//   |  |            X             |    |               x          |   |           xxxxx          |   |       ...
//   |  |            X             |    |              x           |   |               x          |   |       ...
//   |  |            X             |    |            x             |   |                x         |   |       ...
//   |  |            X             |    |           x              |   |                 x        |   |       ...
//   |  |            X             |    |          x               |   |                 x        |   |       ...
//   |  |         XXXXXXX          |    |        xxxxxxxxxx        |   |          x     x         |   |       ...
//   |  |                          |    |                          |   |           xxxxx          |   |       ...
//   |  |                          |    |                          |   |                          |   |       ...
//   |  +--------------------------+    +--------------------------+   +--------------------------+   +------ ...
//   
//  
//   +-------------+
//   | C1 C2 C3 C4 |   Player's hand
//   +-------------+
//   Top left:  C1 - 2,6 -> 28,32 
//   Top left:  C2 - 32,6 -> 58, 32
//   Top left:  C3 - 62,6 -> 88, 32
//   Top left:  C4 - 92,6 -> 118, 32
//  Not quite right.

// +--------------------------------------------------------------------------------------------------------------------------------+
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// |                                                                                                                                |
// +--------------------------------------------------------------------------------------------------------------------------------+




// Different types of cards in the game
#define CARD_NONE 0
#define CARD_PLUS1 1
#define CARD_PLUS2 2
#define CARD_PLUS3 3
#define CARD_PLUS4 4
#define CARD_PLUS5 5
#define CARD_PLUS6 6
#define CARD_MINUS1 7
#define CARD_MINUS2 8
#define CARD_MINUS3 9
#define CARD_MINUS4 10
#define CARD_MINUS5 11
#define CARD_MINUS6 12

#define CARD_REGULARS 12

#define CARD_PLUSORMINUS1 13
#define CARD_PLUSORMINUS2 14
#define CARD_PLUSORMINUS3 15
#define CARD_PLUSORMINUS4 16
#define CARD_PLUSORMINUS5 17
#define CARD_PLUSORMINUS6 18
#define CARD_PLUSORMINUS1Or2 19
#define CARD_FLIP2SAND4S 20
#define CARD_DOUBLE 21
#define CARD_LAST 22

#define SIDE_DECK_SIZE 10
#define HAND_SIZE 4
#define MAX_ROUND_CARDS 7

#define PLAYER_HUMAN 0
#define PLAYER_COMPUTER 1

// Game state
uint16_t sideDeck[2][SIDE_DECK_SIZE];
uint16_t hand[2][HAND_SIZE];
uint8_t handsWonHuman = 0;
uint8_t handsWonComputer = 0;

uint16_t cardsPlayed[2][MAX_ROUND_CARDS];


Timer timer;

uint16_t CARD_COLOR_MAP[7] = {ST7735_RED, ST7735_GREEN, ST7735_CYAN, ST7735_MAGENTA, ST7735_YELLOW, ST7735_BLUE, ST7735_WHITE};

Rtttl soundPlayer;

// A structure that encapsulates what a card looks like.  
int CARD_SHAPES[1][2][8] = {
// Upright simple card
{
 { 0b01111110,
   0b10000001,
   0b10000001,
   0b10000001,
   0b10000001,
   0b10000001,
   0b11111111},
 { 0b11111110,
   0b10000001,
   0b10000001,
   0b10000001,
   0b10000001,
   0b10000001,
   0b11111110},
}
};

bool isCardBitSet(uint8_t cardShellType, uint8_t cardRotation, int x, int y) { 
  byte bitRow = CARD_SHAPES[cardShellType][cardRotation][y];
  return (1 << x) & bitRow;
}

void drawCardShell(uint8_t cardShellType, uint8_t cardRotation, uint8_t windowX, uint8_t windowY, uint16_t color) {
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      if (isCardBitSet(cardShellType, cardRotation, x, y)) {
        Pyxa.drawPixel(windowX + x, windowY + y, color);
      }
    }
  }
}

void drawCard(uint8_t cardType, uint8_t cardRotation, uint8_t windowX, uint8_t windowY, uint16_t color) {
  drawCardShell(0, cardRotation, windowX, windowY, color);
  Pyxa.setTextColor(color);
  Pyxa.setTextSize(0);
  Pyxa.setCursor(windowX + 2, windowY + 7);
  switch (cardType) { 
    case CARD_PLUS1: Pyxa.print( "+1" );break;
    case CARD_PLUS2: Pyxa.print( "+2" );break;
    case CARD_PLUS3: Pyxa.print( "+3" );break;
    case CARD_PLUS4: Pyxa.print( "+4" );break;
    case CARD_PLUS5: Pyxa.print( "+5" );break;
    case CARD_PLUS6: Pyxa.print( "+6" );break;
    case CARD_MINUS1: Pyxa.print( "-1" );break;
    case CARD_MINUS2: Pyxa.print( "-2" );break;
    case CARD_MINUS3: Pyxa.print( "-3" );break;
    case CARD_MINUS4: Pyxa.print( "-4" );break;
    case CARD_MINUS5: Pyxa.print( "-5" );break;
    case CARD_MINUS6: Pyxa.print( "-6" );break;
    case CARD_PLUSORMINUS1: Pyxa.print( "+/-1" );break;
    case CARD_PLUSORMINUS2: Pyxa.print( "+/-2" );break;
    case CARD_PLUSORMINUS3: Pyxa.print( "+/-3" );break;
    case CARD_PLUSORMINUS4: Pyxa.print( "+/-4" );break;
    case CARD_PLUSORMINUS5: Pyxa.print( "+/-5" );break;
    case CARD_PLUSORMINUS6: Pyxa.print( "+/-6" );break;
    case CARD_PLUSORMINUS1Or2: Pyxa.print( "+/- 1-2" );break;
    case CARD_FLIP2SAND4S: Pyxa.print( "F2/4" );break;
    case CARD_DOUBLE: Pyxa.print( "*2" );break;
    case CARD_NONE:
      break;
  }

}

void setup() {
  Serial.begin(9600);
  soundPlayer.begin(A1);

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
  
  initilizeGame();
  start();
}

void initializeSideDeck(uint8_t player) {
  // For now the side deck is a random set of cards.
  for(uint8_t i = 0; i < SIDE_DECK_SIZE; i++) { 
    sideDeck[player][i] = random(CARD_LAST);
  }
}

void initializeHand(uint8_t player) {
  // Jason :: TODO:  Allow the player to chose their hand cards
  for(uint8_t i = 0; i < HAND_SIZE; i++) { 
    hand[player][i] = sideDeck[player][random(SIDE_DECK_SIZE)];
  }
}

void initilizeGame() {
  // Initialize the side deck as well as the hand
  initializeSideDeck(PLAYER_HUMAN);
  initializeHand(PLAYER_HUMAN);
  initializeSideDeck(PLAYER_COMPUTER);
  initializeHand(PLAYER_COMPUTER);

  handsWonHuman = 0;
  handsWonComputer = 0;

  initializeRound();
}

void initializeRound() {
  for(uint8_t i = 0; i < MAX_ROUND_CARDS; i++) { 
    cardsPlayed[PLAYER_HUMAN][i] = CARD_NONE;
    cardsPlayed[PLAYER_COMPUTER][i] = CARD_NONE;
  }
}

void drawScoreScreen() {
  Pyxa.drawRect(0,120,SW-2,SH-2,Pyxa.color565(193,205,205));

  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.setTextSize(0);
  Pyxa.setCursor(125, 10);
  Pyxa.print("You:");

  Pyxa.setCursor(125, 50);
  Pyxa.setTextColor(ST7735_YELLOW);
  Pyxa.print(handsWonHuman);

  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.setTextSize(0);
  Pyxa.setCursor(125, 70);
  Pyxa.print("Bad guy:");

  Pyxa.setCursor(125, 110);
  Pyxa.setTextColor(ST7735_YELLOW);
  Pyxa.print(handsWonComputer);
}

int randomCard(bool regularCard) {
  if (regularCard) { 
    return random(CARD_REGULARS);
  } else {
    // Jason :: TODO:  The cards should not all have equal probability
    return random(CARD_LAST);
  }
}

int placeIntoPlay(uint8_t player, uint16_t card) {
  // Jason :: TODO:  I have to handle card effects here as well.
  uint8_t result;
  for(result = 0; result < MAX_ROUND_CARDS; result++) { 
    if (cardsPlayed[player][result] == CARD_NONE) { 
      break;
    }
  }
  cardsPlayed[player][result] = card;
  return result;
}

bool doTurn(int player) { 
  int cardHuman = randomCard();
  int index = placeIntoPlay(player, cardHuman);
  drawPlayCard(player, index);

  bool handCardPlayed = false;
  while (true) {
    int action = getNextAction(player, handCardPlayed);
    if (action == STAND) { 
      return true;
    } else if (action == END_TURN) { 
      return false;
    } else { 
      handCardPlayed = true;
      int whichHandCard = selectHandCard(player);
      index = placeIntoPlay(hand[player][whichHandCard]);
      hand[player][whichHandCard] = CARD_NONE; 

      drawHandCard(player, whichHandCard);
      drawPlayCard(player, index);
    }
  }
}

int computePlaySum(int player) {
  int sum = 0;
  for(uint8_t i = 0; i < MAX_ROUND_CARDS; i++) { 
    int cardValue = 0; 
    int doubleCardValueIndex = -1;
    switch (cardsPlayed[player][i]) { 
      case CARD_PLUS1: cardValue = 1; break;

      case CARD_PLUS2: cardValue = 2; break;
      case CARD_PLUS3: cardValue = 3; break;
      case CARD_PLUS4: cardValue = 4; break;
      case CARD_PLUS5: cardValue = 5; break;
      case CARD_PLUS6: cardValue = 6; break;
      case CARD_MINUS1: cardValue = -1; break;
      case CARD_MINUS2: cardValue = -2; break;
      case CARD_MINUS3: cardValue = -3; break;
      case CARD_MINUS4: cardValue = -4; break;
      case CARD_MINUS5: cardValue = -5; break;
      case CARD_MINUS6: cardValue = -6; break;

      case CARD_NONE:
      case CARD_PLUSORMINUS1:
      case CARD_PLUSORMINUS2:
      case CARD_PLUSORMINUS3:
      case CARD_PLUSORMINUS4:
      case CARD_PLUSORMINUS5:
      case CARD_PLUSORMINUS6:
      case CARD_PLUSORMINUS1Or2:
      case CARD_FLIP2SAND4S:
                        break;

      case CARD_DOUBLE:
                        doubleCardValueIndex = i+1;
                        break;
    }
    if (doubleCardValueIndex == i) { 
      cardValue = cardValue * 2;
    }
    sum += cardValue;
  }
  return sum;
}

void loop() {
  if (handsWonHuman == 3 || handsWonComputer == 3) { 
    over();
  }

  bool standing[2] = { false, false };
  bool humanWon = false;

  initializeRound();
  drawRound();

  while (true) { 
    if( !standing[HUMAN_PLAYER] ) { 
      standing[HUMAN_PLAYER] = doTurn(HUMAN_PLAYER);
    }

    int humanSum = computePlaySum(HUMAN_PLAYER);
    if (humanSum == 20) { 
      humanWon = true;
      break;
    }
    
    if( !standing[COMPUTER_PLAYER] ) { 
      standing[COMPUTER_PLAYER] = doTurn(HUMAN_PLAYER);
    }
    int computerSum = computePlaySum(COMPUTER_PLAYER);
    if (computerSum == 20) { 
      humanWon = false;
      break;
    }

    if (standing[HUMAN_PLAYER] && standing[COMPUTER_PLAYER]) { 
      if (humanSum > computerSum) { 
        humanWon = true;
      } else { 
        humanWon = false;
      }
      break;
    }
  }
 
  if (humanWon) { 
     handsWonHuman += 1;
  } else { 
     handsWonComputer += 1;
  }
  drawWinnerAndUpdateScore(humanWon);
}

void over() {
  Pyxa.fillScreen(ST7735_BLACK);
  Pyxa.setTextColor(ST7735_RED);
  Pyxa.setTextSize(2);
  Pyxa.setCursor( (SW/2) - (9*6), (SH/2) - 4);
  Pyxa.println("GAME OVER");
  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.setTextSize(0);

  Pyxa.setCursor( 35, (SH/2) - 46);
  Pyxa.print("You: ");
  Pyxa.print(handsWonHuman);

  Pyxa.setCursor( 35, (SH/2) - 26);
  Pyxa.print("Computer: ");
  Pyxa.print(handsWonComputer);

  Pyxa.setCursor( 32, 105);
  Pyxa.println("Press -A-");

  waitForAButtonDownThenUp();
  initilizeGame();
}

void waitForAButtonDownThenUp() {
  waitForButtonDownThenUp(A_BUTTON);
}

void waitForButtonDownThenUp(int pin) {
  while (1) {
    if (digitalRead(pin) == 0) { 
      delay(10);
      while( digitalRead(pin) == 0 ) { 
        delay(5);
      }
      delay(10);
      break;
    }
  }
}
 
void start() {
  // Draw start screen
  Pyxa.fillScreen(ST7735_BLACK);
  Pyxa.setTextColor(ST7735_RED);
  Pyxa.setTextSize(2);
  Pyxa.setCursor(30, 50);
  Pyxa.println("Pazaak");

  Pyxa.setCursor( 10, 105);
  Pyxa.println("Press -A- to Start");
  waitForAButtonDownThenUp();
  soundPlayer.play_P(gameBeginSound);
}
