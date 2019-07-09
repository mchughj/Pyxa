
//
// Tetris
//

// 7/5/2019
//
// Originally taken from sample code for Pxya.  
//
// Added a level tracker, different speed settings per level, different line count
// requirements to move between levels, and the 'ghost' image of where your current
// brick will land (and orientation) if it were to go all the way to the bottom in
// the current configuration and position.
//
// Fixed a few bugs and added sound.
//
// Finally, I tightened up some of the memory usage here, for example bits
// rather than bytes for capturing what each piece looks like.  This was
// necessary due to the tight memory constraints.
//
// I would have liked to have made the sounds play async but the only RTTTL
// library readily available uses a ton of memory and I didn't want to 
// change that library or compact that as part of this.  Plus many of the
// sounds are very short.
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
const char tetrisStartSond[] PROGMEM = "Tetris:d=4,o=5,b=150:e6,8b,8c6,8d6,16e6,16d6,8c6,8b,a,8a,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,2a,8p,d6,8f6,a6,8g6,8f6,e6,8e6,8c6,e6,8d6,8c6,b,8b,8c6,d6,e6,c6,a,a";
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

// A note that will be used for the buzzer
#define LA3 220.00
#define BPM 120
#define Q 1200/BPM 

// Screen size in pixels
#define SW   128 // Screen width
#define SH   160 // Screen height

// Game level settings
#define MAX_LEVEL 8
// Level delays are the number of milliseconds between a tetris block moving
// from the current height to the next lower height.
unsigned int levelDelays[MAX_LEVEL] = {
  550,
  500,
  450,
  400,
  350,
  300,
  200,
  100,
};

// Lines clear are the number of lines that must be cleared, total (and not
// just for the current level), in order to move to the next level.
uint8_t linesClearedForNextLevel[MAX_LEVEL] = {
  5,
  10,
  15,
  20,
  25,
  35,
  45,
  55,
};
uint8_t currentGameLevel = 0;

Timer timer;

// Define actions in the game
#define MOVE_LEFT   0
#define MOVE_RIGHT  1
#define MOVE_DOWN   2
#define ROTATE      3
#define SPAWN       4

// Define playfield
#define CELLS_WIDE   11
#define CELLS_TALL   20
#define CELL_SIZE     8
#define SPAWN_POS_X   6
#define SPAWN_POS_Y   0
#define VOID_CELL   ST7735_BLACK

// Define bricks

#define BRICK_I 0
#define BRICK_S 1
#define BRICK_Z 2
#define BRICK_J 3
#define BRICK_O 4
#define BRICK_L 5
#define BRICK_T 6

// Define rotations
#define ROT_0   0
#define ROT_90  1
#define ROT_180 2
#define ROT_270 3

// Playfield Object

uint16_t Playfield[CELLS_WIDE][CELLS_TALL];
int8_t currentBrickPos[2]     = { SPAWN_POS_X, SPAWN_POS_Y };
uint8_t currentBrickType      = BRICK_L;
uint8_t currentBrickRotation  = ROT_0;
uint8_t currentBrickColor     = 0;
uint8_t nextBrickColor        = 1;
uint8_t nextBrickType         = BRICK_L;
uint8_t nextBrickRotation     = ROT_0;
uint8_t retryForceMove        = 0;
uint8_t linesCleaned          = 0;

// Color Map

uint16_t BRICK_COLOR_MAP[7] = {ST7735_RED, ST7735_GREEN, ST7735_CYAN, ST7735_MAGENTA, ST7735_YELLOW, ST7735_BLUE, ST7735_WHITE};

Rtttl soundPlayer;

// A structure that encapsulates what each brick looks like and for each orientation.
// This is a bit structure where a 0 means that the brick doesn't have a block at that
// position and 1 means it does.
byte BRICK_MAP[7][4][4] = {

// BRICK_I
{
 { 0b1111,
   0b0000,
   0b0000,
   0b0000,
 },

 { 0b0100,
   0b0100,
   0b0100,
   0b0100 },

 { 0b1111,
   0b0000,
   0b0000,
   0b0000 },

 { 0b0100,
   0b0100,
   0b0100,
   0b0100 }
},

// BRICK_S
{
 { 0b0110,
   0b1100,
   0b0000,
   0b0000 },

 { 0b0100,
   0b0110,
   0b0010,
   0b0000 },

 { 0b0110,
   0b1100,
   0b0000,
   0b0000 },

 { 0b0100,
   0b0110,
   0b0010,
   0b0000 }
},

// BRICK_Z
{
 { 0b1100,
   0b0110,
   0b0000,
   0b0000 },

 { 0b0010,
   0b0110,
   0b0100,
   0b0000 },

 { 0b1100,
   0b0110,
   0b0000,
   0b0000 },

 { 0b0010,
   0b0110,
   0b0100,
   0b0000 }
},

// BRICK_J
{
 { 0b0110,
   0b0100,
   0b0100,
   0b0000 },

 { 0b1000,
   0b1110,
   0b0000,
   0b0000 },

 { 0b1110,
   0b0010,
   0b0000,
   0b0000 },

 { 0b0010,
   0b0010,
   0b0110,
   0b0000 }
},


// BRICK_O
{
 { 0b0110,
   0b0110,
   0b0000,
   0b0000 },

 { 0b0110,
   0b0110,
   0b0000,
   0b0000 },

 { 0b0110,
   0b0110,
   0b0000,
   0b0000 },

 { 0b0110,
   0b0110,
   0b0000,
   0b0000 }
},

// BRICK_L
{
 { 0b0010,
   0b1110,
   0b0000,
   0b0000 },

 { 0b1110,
   0b1000,
   0b0000,
   0b0000 },

 { 0b0110,
   0b0010,
   0b0010,
   0b0000 },

 { 0b0100,
   0b0100,
   0b0110,
   0b0000 }
},

// BRICK_T
{
 { 0b0100,
   0b1110,
   0b0000,
   0b0000 },

 { 0b0100,
   0b0110,
   0b0100,
   0b0000 },

 { 0b1110,
   0b0100,
   0b0000,
   0b0000 },

 { 0b0010,
   0b0110,
   0b0010,
   0b0000 }
}
};


void drawCell(uint8_t x, uint8_t y, uint16_t color) {
  Pyxa.fillRect(x * CELL_SIZE, y * CELL_SIZE,CELL_SIZE,CELL_SIZE, color);
  Pyxa.drawRect(x * CELL_SIZE, y * CELL_SIZE,CELL_SIZE,CELL_SIZE, VOID_CELL);
}

void drawWall(uint16_t color){
  for (uint8_t y = 0; y < CELLS_TALL; y++) {
    drawCell(CELLS_WIDE,y,color);
  }
}

bool isBrickBitSet(int brickType, int brickRotation, int x, int y) { 
  byte bitRow = BRICK_MAP[brickType][brickRotation][y];
  return (1 << x) & bitRow;
}

void fixTetrisToPlayfield() {
  for (uint8_t y = 0; y < 4; y++) {
    for (uint8_t x = 0; x < 4; x++) {
      if (isBrickBitSet(currentBrickType, currentBrickRotation, x, y)) {
        Playfield[currentBrickPos[0] + x][currentBrickPos[1] + y] = BRICK_COLOR_MAP[currentBrickColor];
      }
    }
  }
}

void drawTetrisPyxa(uint8_t brickType, uint8_t brickRotation, uint8_t xPos, uint8_t yPos, uint16_t color) {
  for (uint8_t y = 0; y < 4; y++) {
    for (uint8_t x = 0; x < 4; x++) {
      if (isBrickBitSet(brickType, brickRotation, x, y)) {
        drawCell(xPos + x, yPos + y, color);
      }
    }
  }
}

int ghostBrickPosY = 0;

/*
 * Given an x and y coordinate for the currently falling tetris brick determine
 * the y ending position of that brick if it were fall all the way to the
 * bottom given the current brick type and rotation.
 *
 */
int findGhostBrickY(int x, int y) { 
  // Simple linear search starting from the current y location.
  // There are a lot more efficient ways to do this but none of them are necessary.
  int result = y;

  Serial.print("ghost;");
  while( collisionDetection(currentBrickRotation, x, result) == false) { 
    result = result + 1;
  } 
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.println(result);
  return result-1;
}

void drawCurrentBrick() {
  // Draw the ghost image of where the brick will land first.
  ghostBrickPosY = findGhostBrickY(currentBrickPos[0], currentBrickPos[1]);
  drawTetrisPyxa(currentBrickType, currentBrickRotation,
                     currentBrickPos[0],ghostBrickPosY,
                     0x514A);

  // Then draw the brick itself.
  drawTetrisPyxa(currentBrickType,currentBrickRotation,
                     currentBrickPos[0],currentBrickPos[1],
                     BRICK_COLOR_MAP[currentBrickColor]);
}

void eraseCurrentBrick() {
  // Erase the ghost image first
  drawTetrisPyxa(currentBrickType,currentBrickRotation,
                     currentBrickPos[0],ghostBrickPosY,
                     VOID_CELL);
  // Erase the actual brick second
  drawTetrisPyxa(currentBrickType,currentBrickRotation,
                     currentBrickPos[0],currentBrickPos[1],
                     VOID_CELL);
}

bool collisionDetection(uint8_t action) {
  return collisionDetectionWithAction(currentBrickPos[0], currentBrickPos[1], action);
}

bool collisionDetectionWithAction(int x, int y, uint8_t action) {
  int8_t futureBrickPos[2];
  futureBrickPos[0] = x;
  futureBrickPos[1] = y;
  uint8_t futureBrickRotation = currentBrickRotation;

  switch (action) {
    case MOVE_LEFT: {
      futureBrickPos[0] = futureBrickPos[0] - 1;
    }
    break;
    case MOVE_RIGHT: {
      futureBrickPos[0] = futureBrickPos[0] + 1;
    }
    break;
    case MOVE_DOWN: {
      futureBrickPos[1] = futureBrickPos[1] + 1;
    }
    break;
    case ROTATE: {
      futureBrickRotation = (futureBrickRotation + 1) % 4;
    }
    break;
    case SPAWN: {
      // Do nothing
    }
    break;
    default:
      break;
  }
  return collisionDetection(futureBrickRotation, futureBrickPos[0], futureBrickPos[1]);
}

bool collisionDetection(uint8_t brickRotation, int posX, int posY) {
  for (uint8_t y = 0; y < 4; y++) {
    for (uint8_t x = 0; x < 4; x++) {
      if (isBrickBitSet(currentBrickType, brickRotation, x, y)) {
        // Check Playfield collision
        if (Playfield[posX + x][ posY + y] != VOID_CELL ) {
          return true;
        }
        // Check Left/Right border collision
        if ( posX + x < 0 || posX + x >= CELLS_WIDE ) {
          return true;
        }
        // Check bottom border collision
        if ( posY + y >= CELLS_TALL) {
          return true;
        }
      }
    }
  }

  return false;
}

void moveLeft() {
  if (collisionDetection(MOVE_LEFT) == false) {
    eraseCurrentBrick();
    currentBrickPos[0] = currentBrickPos[0] - 1;
    drawCurrentBrick();
    tone(A1,LA3,Q);
  }
}

void moveRight() {
  if (collisionDetection(MOVE_RIGHT) == false) {
    eraseCurrentBrick();
    currentBrickPos[0] = currentBrickPos[0] + 1;
    drawCurrentBrick();
    tone(A1,LA3,Q);
  }
}

bool moveDown()
{
  if (collisionDetection(MOVE_DOWN) == false) {
    eraseCurrentBrick();
    currentBrickPos[1] = currentBrickPos[1] + 1;
    drawCurrentBrick();
    tone(A1,LA3,Q);
    return true;
  }
  return false;
}

void rotate() {
  if (collisionDetection(ROTATE) == false) {
    eraseCurrentBrick();
    currentBrickRotation = (currentBrickRotation + 1) % 4;
    drawCurrentBrick();
    tone(A1,LA3,Q);
  }
}

void clearPlayfield() {
  // Initialize Playfield with void cells
  for (uint8_t x = 0; x < CELLS_WIDE; x++) {
    for (uint8_t y = 0; y < CELLS_TALL; y++) {
      Playfield[x][y] = VOID_CELL;
    }
  }
}

void drawPlayfield() {
  for (uint8_t x = 0; x < CELLS_WIDE; x++) {
    for (uint8_t y = 0; y < CELLS_TALL; y++) {
      drawCell(x,y,Playfield[x][y]);
    }
  }
}

void setup() {
  
  // Port manipulation for A and B
  DDRD &= ~(1<<PD5);
  DDRD &= ~(1<<PD6);

  // Port manipulation for Direction Buttons
  DDRC &= ~(1<<PD2);
  DDRC &= ~(1<<PD3);
  DDRC &= ~(1<<PD4);
  DDRC &= ~(1<<PD5);

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
  initializeTimer();
}

void initializeTimer() {
  // Timer Setup
  timer.after(levelDelays[currentGameLevel],updateWorld);
  timer.every(60,controller);
}

void initilizeGame() {

  linesCleaned = 0;
  currentGameLevel = 0;
 
  start();

  // Spawn random Brick
  randomSeed(analogRead(0));
  nextBrickType      = random(7);
  nextBrickRotation  = random(4);
  
  clearPlayfield();
  drawSideScreen();
  spawnBrick();

  drawPlayfield();
  drawCurrentBrick();
}

void drawSideScreen(){
  drawWall(Pyxa.color565(193,205,205));
  Pyxa.fillRect(92,0,40,160,VOID_CELL);
  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.setTextSize(0);
  Pyxa.setCursor( 100, 20);
  Pyxa.print("NEXT");

  Pyxa.setCursor( 95, 80);
  Pyxa.print("SCORE");
  Pyxa.setCursor( 100, 95);  
  Pyxa.setTextColor(ST7735_YELLOW);
  Pyxa.print(linesCleaned*100); 

  Pyxa.setCursor( 95, 120);
  Pyxa.setTextColor(ST7735_WHITE);
  Pyxa.print("LEVEL");
  Pyxa.setCursor( 100, 135);  
  Pyxa.setTextColor(ST7735_YELLOW);
  Pyxa.print(currentGameLevel+1);
}

void drawFutureBrick(){
  drawTetrisPyxa(nextBrickType, 3, 12, 5, BRICK_COLOR_MAP[nextBrickColor]);
}

long m_lastRotationTime = 0;
long m_lastLeftTime = 0;
long m_lastRightTime = 0;

void controller() {
  if (!(PINC & (1<<PD4)) ) {
    if (millis() - m_lastRightTime > 200) {
      m_lastRightTime = millis();
      moveRight();
    }
  }
  if (!(PINC & (1<<PD2)) ) {
    if (millis() - m_lastLeftTime > 200) {
      m_lastLeftTime = millis();
      moveLeft();
    }
  }
  if (!(PINC & (1<<PD3)) ) {
    moveDown();
  }
  if (!(PIND & (1<<PD5)) ) {
    if (millis() - m_lastRotationTime > 200) {
      m_lastRotationTime = millis();
      rotate();
    }
  }
}
  
void spawnBrick() {
  currentBrickPos[0]    = SPAWN_POS_X;
  currentBrickPos[1]    = SPAWN_POS_Y;
  currentBrickType      = nextBrickType;
  currentBrickRotation  = nextBrickRotation;
  nextBrickType      = random(7);
  nextBrickRotation  = random(4);
  currentBrickColor  = nextBrickColor;
  nextBrickColor  = nextBrickType;
  Pyxa.fillRect(92,40,30,40,VOID_CELL);
  drawFutureBrick();
}

void clearLineOnPlayfield(uint8_t line) {
  for (int8_t y = line; y >= 1; --y) {
    for (uint8_t x = 0; x < CELLS_WIDE; x++) {
      Playfield[x][y] = Playfield[x][y - 1];
    }
  }
}

uint8_t checkLinesCleaned() {
  uint8_t lineCleanedCount = 0;
  for (int8_t y = 0; y < CELLS_TALL; y++) {
    bool clearLine = true;
    for (uint8_t x = 0; x < CELLS_WIDE; x++) {
      if (Playfield[x][y] == VOID_CELL) {
        clearLine = false;
        break;
      }
    }
    if (clearLine) {
      soundPlayer.play_P(clearLineSound);  
      clearLineOnPlayfield(y);
      lineCleanedCount++;

    }
    clearLine = true;
  }
  return lineCleanedCount;
}

void updateWorld() {
  if (moveDown() == true) { // Try to go down
    retryForceMove = 0;
  } else {
    if (++retryForceMove > 2 ) { // block tetris on Playfield
      fixTetrisToPlayfield();
      soundPlayer.play_P(hitBottomSound);  

      // check lines completed
      uint8_t lastLinesCleaned = checkLinesCleaned();
      if (lastLinesCleaned > 0) {
        drawPlayfield();
        linesCleaned += lastLinesCleaned;

        if (linesCleaned > linesClearedForNextLevel[currentGameLevel]) {
           if (currentGameLevel+1<MAX_LEVEL) { 
             currentGameLevel += 1;
           }
        }

        Pyxa.fillRect(100,95,30,20,VOID_CELL);
        Pyxa.setCursor(100, 95);
        Pyxa.setTextColor(ST7735_YELLOW);
        Pyxa.print(linesCleaned*100);        

        Pyxa.fillRect(100,135,30,20,VOID_CELL);
        Pyxa.setCursor(100, 135);
        Pyxa.setTextColor(ST7735_YELLOW);
        Pyxa.print(currentGameLevel+1);
      } 

      // Prepare spawn new brick

      spawnBrick();

      drawCurrentBrick();

      if (((Playfield[7][0] != VOID_CELL)|( Playfield[8][0] != VOID_CELL )|(Playfield[9][0] != VOID_CELL)|(Playfield[10][0] != VOID_CELL))&&
      ((Playfield[7][1] != VOID_CELL)|( Playfield[8][1] != VOID_CELL )|(Playfield[9][1] != VOID_CELL)|(Playfield[10][1] != VOID_CELL))&&
      ((Playfield[7][2] != VOID_CELL)|( Playfield[8][2] != VOID_CELL )|(Playfield[9][2] != VOID_CELL)|(Playfield[10][2] != VOID_CELL))&&
      ((Playfield[7][3] != VOID_CELL)|( Playfield[8][3] != VOID_CELL )|(Playfield[9][3] != VOID_CELL)|(Playfield[10][3] != VOID_CELL))) { 
        over();
      }
    }
  }
  timer.after(levelDelays[currentGameLevel],updateWorld);
}

void loop() {
  timer.update();
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
  Pyxa.print("Score: ");
  Pyxa.print(linesCleaned*100);

  Pyxa.setCursor( 35, (SH/2) - 26);
  Pyxa.print("Level: ");
  Pyxa.print(currentGameLevel+1);

  Pyxa.setCursor( 32, 105);
  Pyxa.println("Press -A-");

  waitForAButtonDownThenUp();
  initilizeGame();
  tone(A1,LA3,Q);
}


void waitForAButtonDownThenUp() {
  while (1) {
    // Wait for push button A
    if ( !(PIND & (1<<PD5)) ) {
      break;
    }
  }
  unsigned long int startTime = millis();
  while( millis() - startTime < 20 ) {}

  while (1) {
    // Wait for push button A
    if ( (PIND & (1<<PD5)) ) {
      break;
    }
  }
}
 
void start() {
  // Draw start screen
  Pyxa.fillScreen(ST7735_BLACK);
  Pyxa.fillRect(7, 55, 21, 7, ST7735_RED);
  Pyxa.fillRect(14, 62, 7, 28, ST7735_RED);
  Pyxa.fillRect(30, 55, 7, 35, ST7735_MAGENTA);
  Pyxa.fillRect(37, 55, 7, 7, ST7735_MAGENTA);
  Pyxa.fillRect(37, 70, 7, 5, ST7735_MAGENTA);
  Pyxa.fillRect(37, 83, 7, 7, ST7735_MAGENTA);
  Pyxa.fillRect(46, 55, 21, 7, ST7735_YELLOW);
  Pyxa.fillRect(53, 62, 7, 28, ST7735_YELLOW);
  Pyxa.fillRect(69, 55, 7, 35, ST7735_GREEN);
  Pyxa.fillRect(76, 55, 7, 7, ST7735_GREEN);
  Pyxa.fillRect(76, 69, 7, 7, ST7735_GREEN);
  Pyxa.fillRect(83, 62, 6, 7, ST7735_GREEN);
  Pyxa.fillRect(83, 76, 6, 14, ST7735_GREEN);
  Pyxa.fillRect(91, 55, 7, 35, ST7735_WHITE);
  Pyxa.fillRect(100, 55, 21, 7, ST7735_CYAN);
  Pyxa.fillRect(100, 62, 7, 7, ST7735_CYAN);
  Pyxa.fillRect(107, 69, 7, 7, ST7735_CYAN);
  Pyxa.fillRect(114, 76, 7, 7, ST7735_CYAN);
  Pyxa.fillRect(100, 83, 21, 7, ST7735_CYAN);

  Pyxa.fillRect(6, 51, 116, 2, ST7735_WHITE);
  Pyxa.fillRect(6, 92, 116, 2, ST7735_WHITE);

  Pyxa.setCursor( 10, 105);
  Pyxa.println("Press -A- to Start");

  soundPlayer.play_P(tetrisStartSound);
  waitForAButtonDownThenUp();
  soundPlayer.play_P(gameBeginSound);
}
