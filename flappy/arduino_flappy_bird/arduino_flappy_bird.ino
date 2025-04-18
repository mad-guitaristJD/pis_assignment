


#include <LEDMatrixDriver.hpp>
#include "/Users/jaideep/Desktop/pis_assignment/game_selector/fontmatrix.h"

#define MARQUEE_ANIM_DELAY  100     
#define GAME_FRAME_RATE     400     
#define MUTE                1       

//dot matrix connections
const uint8_t LEDMATRIX_CS_PIN = 9;
const int LEDMATRIX_SEGMENTS = 1;
const int LEDMATRIX_WIDTH  = LEDMATRIX_SEGMENTS * 8;
const int MID_POS = 4;              
const int INIT_BIRD_POS = 3;        
const int JUMP_HEIGHT = 1;          
const int MAX_JUMP = 2;             


char text[] = "FLAPPY BIRD PRESS BUTTON TO START";


LEDMatrixDriver lmd(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);


int buttonPin = 2;
int buzzerPin = 5;

int timeLine = 0;                   
int birdHeight = INIT_BIRD_POS;     
int holePosition = 0;               
bool falling = true;                
int riseCount = 0;                  
bool startGame = false;             
int marquee_pos_x = 0;              


void drawBird(int x, int y) {
  lmd.setPixel(x, y, true);
}


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

void drawString(char* text, int len, int x, int y )
{
  for( int idx = 0; idx < len; idx ++ )
  {
    int c = text[idx] - 32;

    if( x + idx * 8  > LEDMATRIX_WIDTH )
      return;

    if( 8 + x + idx * 8 > 0 ){
      drawSprite( font[c], x + idx * 8, y, 8, 8 );
    }
     
  }
}


void jumpBird(){
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  if (interrupt_time - last_interrupt_time > 200){
      if(!startGame){
        startGame = true;
      }
      falling = !falling;  
  }
  last_interrupt_time = interrupt_time; 
}


void gameOver(){
  #ifdef MUTE
  gameOverBuzz();
  #endif
  
  for(int j = 0; j < 3; j++){
    lmd.clear();
    for (int i = 0; i < 8; i++) {
      lmd.setPixel(i, i, true);                 
      lmd.setPixel(i, 7 - i, true);             
      delay(100);  
    }
    lmd.display();
    delay(500);
  }

  
  birdHeight = 3;
  lmd.clear();
  startGame = false;
  timeLine = 0;
  falling = true;
  holePosition = random(1, 3);
}


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


void showIntro()
{
  int len = strlen(text);
  drawString(text, len, marquee_pos_x, 0);
  lmd.display();
  delay(MARQUEE_ANIM_DELAY);
  if( --marquee_pos_x < len * -8 ) {
    marquee_pos_x = LEDMATRIX_WIDTH;
  }
}

void drawSprite( byte* sprite, int x, int y, int width, int height )
{
  byte mask = B10000000;

  for( int iy = 0; iy < height; iy++ )
  {
    for( int ix = 0; ix < width; ix++ )
    {
      lmd.setPixel(x + ix, y + iy, (bool)(sprite[iy] & mask ));

      mask = mask >> 1;
    }

    mask = B10000000;
  }
}


void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), jumpBird, CHANGE);
  randomSeed(analogRead(0));
  lmd.setEnabled(true);
  lmd.setIntensity(2);
  holePosition = random(1, 3);
  drawBird(birdHeight,MID_POS);
}


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
