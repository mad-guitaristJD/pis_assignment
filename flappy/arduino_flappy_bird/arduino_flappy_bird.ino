


#include <LEDMatrixDriver.hpp>
#include "/Users/jaideep/Desktop/pis_assignment/game_selector/fontmatrix.h"

#define MARQUEE_ANIM_DELAY  100     // Marquee speed (lower nubmers = faster)
#define GAME_FRAME_RATE     400     // Game frame rate (lower nubmers = faster)
#define MUTE                1       // Comment out if sound is to be disabled

//dot matrix connections
const uint8_t LEDMATRIX_CS_PIN = 9;
const int LEDMATRIX_SEGMENTS = 1;
const int LEDMATRIX_WIDTH  = LEDMATRIX_SEGMENTS * 8;
const int MID_POS = 4;              //bird x position at the start (also middle of matrix)
const int INIT_BIRD_POS = 3;        //bird y position at the start
const int JUMP_HEIGHT = 1;          //how many dots the bird jumps with every press
const int MAX_JUMP = 2;             //the maximum height the bird jumps before falling

// Marquee text on intro
char text[] = "FLAPPY BIRD PRESS BUTTON TO START";

// The LEDMatrixDriver class instance
LEDMatrixDriver lmd(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);

// Action button. Must be an external interrupt pin
int buttonPin = 2;
int buzzerPin = 5;

int timeLine = 0;                   //variable used to determine the current position of obstacle
int birdHeight = INIT_BIRD_POS;     //variable used to determine the current position of bird
int holePosition = 0;               //variable used to determine the current position of hole in obstacle
bool falling = true;                //variable to indicate if bird is falling or rising
int riseCount = 0;                  //variable used to count bird position as it jumps
bool startGame = false;             //variable flag to start the game
int marquee_pos_x = 0;              //variable used to set marquee text position

/**
 * This function draws the bird to the given position.
 */
void drawBird(int x, int y) {
  lmd.setPixel(x, y, true);
}

/**
 * This function draws an obstacle given the position of the hole.
 */
void drawObstacle(int hole)
{
  int height = 0;
  while(height != hole){
    lmd.setPixel(height, timeLine, true);
    height++;
  }
  height = height + 3;
  while(height != 8){
    lmd.setPixel(height, timeLine, true);
    height++;
  }
  
}

/**
 * This function draws a string of the given length to the given position.
 */
void drawString(char* text, int len, int x, int y )
{
  for( int idx = 0; idx < len; idx ++ )
  {
    int c = text[idx] - 32;

    // stop if char is outside visible area
    if( x + idx * 8  > LEDMATRIX_WIDTH )
      return;

    // only draw if char is visible
    if( 8 + x + idx * 8 > 0 ){
      drawSprite( font[c], x + idx * 8, y, 8, 8 );
    }
     
  }
}

/**
 * Function that makes the bird jump
  */ 
void jumpBird(){
  //handle button debounce
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200){
      if(!startGame){
        startGame = true;
      }
      falling = !falling;  
  }
  last_interrupt_time = interrupt_time; 
}

/**
 * Function called when bird hits obstacle or the ground
  */ 
void gameOver(){
  //game over sound
  #ifdef MUTE
  gameOverBuzz();
  #endif
  
  //draw an X on matrix
  for(int j = 0; j < 3; j++){
    lmd.clear();
    for (int i = 0; i < 8; i++) {
      lmd.setPixel(i, i, true);                 // Diagonal from top-left to bottom-right
      lmd.setPixel(i, 7 - i, true);              // Diagonal from top-right to bottom-left
      delay(100);  // Delay for better visibility (optional)
    }
    lmd.display();
    delay(500);
  }

  //reset bird position and obstacles
  birdHeight = 3;
  lmd.clear();
  startGame = false;
  timeLine = 0;
  falling = true;
  holePosition = random(1, 3);
}

/**
 * Function for game over sound
 */
void gameOverBuzz()
{
  int buzzLoop = 5;
  while(buzzLoop != 0) {
    analogWrite(buzzerPin, 200);
    delay(200);
    analogWrite(buzzerPin, 0);
    delay(200);
    buzzLoop--;
  }
}

/**
 * Function that shows running text intro
  */ 
void showIntro()
{
  // Draw the text to the current position
  int len = strlen(text);
  drawString(text, len, marquee_pos_x, 0);
  
  // In case you wonder why we don't have to call lmd.clear() in every loop: The font has a opaque (black) background...

  // Toggle display of the new framebuffer
  lmd.display();

  // Wait to let the human read the display
  delay(MARQUEE_ANIM_DELAY);

  // Advance to next coordinate
  if( --marquee_pos_x < len * -8 ) {
    marquee_pos_x = LEDMATRIX_WIDTH;
  }
}

/**
 * This draws a sprite to the given position using the width and height supplied (usually 8x8)
 */
void drawSprite( byte* sprite, int x, int y, int width, int height )
{
  // The mask is used to get the column bit from the sprite row
  byte mask = B10000000;

  for( int iy = 0; iy < height; iy++ )
  {
    for( int ix = 0; ix < width; ix++ )
    {
      lmd.setPixel(x + ix, y + iy, (bool)(sprite[iy] & mask ));

      // shift the mask by one pixel to the right
      mask = mask >> 1;
    }

    // reset column mask
    mask = B10000000;
  }
}

/**
 * Everything here is run once
 */
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), jumpBird, CHANGE);
  randomSeed(analogRead(0));
  lmd.setEnabled(true);
  lmd.setIntensity(2);
  //randomize first hole position
  holePosition = random(1, 3);
  //bird always starts at (3, 4)
  drawBird(birdHeight,MID_POS);
}

/**
 * Everything here repeats while power is on
  */ 
void loop() {
  while(!startGame){
    showIntro();
  }
  lmd.clear();
  if(falling){
     birdHeight++; 
  } 
  else{
     birdHeight--;
     riseCount = riseCount + JUMP_HEIGHT;
     if(riseCount == MAX_JUMP){
        falling = true;
        riseCount = 0;
     } 
        
  }
    
  delay(GAME_FRAME_RATE);
  
  drawBird(birdHeight,4);
  drawObstacle(holePosition);
  lmd.display();
  timeLine++;
  if(timeLine == 8){
    timeLine = 0;
    holePosition = random(1, 5);
  }

  int holePlus = holePosition + 1;
  int holePlus2 = holePosition + 2;
  if((birdHeight != holePosition && timeLine == 4 && birdHeight != holePlus && birdHeight != holePlus2) || birdHeight == 7)
  {
    gameOver();
  }

}
