// Program for a 2 button snake game using the MAX7219 LED matrix.

// The MAX7219 uses SPI communication protocol...Hence, import SPI.h library
#include <SPI.h>

// The chip select pin
#define CS 9

// Few necessary registers for configuring the MAX7219 chip
#define DECODE_MODE 9
#define INTENSITY 0x0A
#define SCAN_LIMIT 0x0B
#define SHUTDOWN 0x0C
#define DISPLAY_TEST 0x0F

// Buttons used for controlling the snake
#define left_button 2
#define right_button 3

volatile byte move_left = 0;
volatile byte move_right = 0;

// Varibles for snake
int snake_l = 2;
const int max_len = 15;
int snake[max_len][2];
byte cur_heading = 0;

// Variable for food blob
int blob[2] = { 0, 0 };
int is_eaten = 1;

// The game scene
byte scene[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// A general function to send data to the MAX7219
void SendData(uint8_t address, uint8_t value) {
  digitalWrite(CS, LOW);
  SPI.transfer(address);   // Send address.
  SPI.transfer(value);     //   Send the value.
  digitalWrite(CS, HIGH);  // Finish transfer.
}

// Function to initialize the game variables
void init_game() {
  is_eaten = 1;
  move_left = 0;
  move_right = 0;
  cur_heading = 0;
  snake_l = 2;
  for (int i = 0; i < max_len; i++)
    for (int j = 0; j < 2; j++)
      snake[i][j] = 0;
  snake[max_len - 1][0] = 2;
  snake[max_len - 1][1] = 5;
  snake[max_len - 2][0] = 1;
  snake[max_len - 2][1] = 5;
  refresh_scene();
  while ((move_left || move_right) == 0)
    ;
  move_left = 0;
  move_right = 0;
}

// Function to draw snake on gamescene
void spawn_snake() {
  // If the snake goes out of the scene, it enters from the other side
  for (int j = max_len - snake_l; j < max_len; j++) {
    if (snake[j][0] <= 0)
      snake[j][0] = 8 + snake[j][0];
    else if (snake[j][0] >= 9)
      snake[j][0] = snake[j][0] - 8;
    if (snake[j][1] <= 0)
      snake[j][1] = 8 + snake[j][1];
    else if (snake[j][1] >= 9)
      snake[j][1] = snake[j][1] - 8;

    // Draw the snake on the LED matrix
    scene[snake[j][0] - 1] |= (1 << (snake[j][1] - 1));
  }
}

// Function to update the position and length of the snake
void snake_move() {
  // If snake eats a blob...Increase length
  if (snake[max_len - 1][0] == blob[0] && snake[max_len - 1][1] == blob[1]) {
    is_eaten = 1;
    snake_l += 1;
  }

  // Move each pixel forward
  for (int i = snake_l - 1; i >= 1; i--) {
    snake[max_len - 1 - i][0] = snake[max_len - i][0];
    snake[max_len - 1 - i][1] = snake[max_len - i][1];
  }

  // Move the head according to button input
  if (move_left == 1) {
    if (cur_heading == 0) {
      cur_heading = 1;
      snake[max_len - 1][1] -= 1;
    } else if (cur_heading == 1) {
      cur_heading = 2;
      snake[max_len - 1][0] -= 1;
    } else if (cur_heading == 2) {
      cur_heading = 3;
      snake[max_len - 1][1] += 1;
    } else if (cur_heading == 3) {
      cur_heading = 0;
      snake[max_len - 1][0] += 1;
    }
    move_left = 0;
  } else if (move_right == 1) {
    if (cur_heading == 0) {
      cur_heading = 3;
      snake[max_len - 1][1] += 1;
    } else if (cur_heading == 1) {
      cur_heading = 0;
      snake[max_len - 1][0] += 1;
    } else if (cur_heading == 2) {
      cur_heading = 1;
      snake[max_len - 1][1] -= 1;
    } else if (cur_heading == 3) {
      cur_heading = 2;
      snake[max_len - 1][0] -= 1;
    }
    move_right = 0;
  } else {
    if (cur_heading == 0) {
      snake[max_len - 1][0] += 1;
    } else if (cur_heading == 1) {
      snake[max_len - 1][1] -= 1;
    } else if (cur_heading == 2) {
      snake[max_len - 1][0] -= 1;
    } else if (cur_heading == 3) {
      snake[max_len - 1][1] += 1;
    }
  }
}

// Function to generate a blob and draw the blob on the gamescene
void blob_generator() {
  // If blob is eaten by the snake, generate one
  if (is_eaten) {
    blob[0] = random(1, 9);
    blob[1] = random(1, 9);
  }

  // Draw the blob on the gamescene
  scene[blob[0] - 1] |= (1 << (blob[1] - 1));
  is_eaten = 0;
}

// Function to redraw the gamescene to the LED matrix with updated variables
void refresh_scene() {
  for (int i = 0; i < 8; i++)
    scene[i] = 0x00;
  snake_move();
  spawn_snake();
  blob_generator();
  for (int i = 1; i < 9; i++)
    SendData(i, scene[i - 1]);
}

// Callback for interrupt attached to left button
void update_left() {
  move_left = 1;
}

// Callback for interrupt attached to right button
void update_right() {
  move_right = 1;
}

// Setup function
void setup() {
  // GPIO Configuration
  pinMode(left_button, INPUT_PULLUP);
  pinMode(right_button, INPUT_PULLUP);
  pinMode(CS, OUTPUT);
  
  // SPI configuration
  SPI.setBitOrder(MSBFIRST);     // Most significant bit first
  SPI.begin();                   // Start SPI
  SendData(DISPLAY_TEST, 0x00);  // Finish test mode.
  SendData(DECODE_MODE, 0x00);   // Disable BCD mode.
  SendData(INTENSITY, 0x01);     // Use lowest intensity.
  SendData(SCAN_LIMIT, 0x0f);    // Scan all digits.
  SendData(SHUTDOWN, 0x01);      // Turn on chip.

  // Random seed generation...Uses the noise in the analog channel 0 to create a random seed
  randomSeed(analogRead(0));

  // Attach interrupts to the buttons
  attachInterrupt(digitalPinToInterrupt(left_button), update_left, FALLING);
  attachInterrupt(digitalPinToInterrupt(right_button), update_right, FALLING);

  // Enable interrupt
  sei();

  // Start the game
  init_game();
}

void loop() {
  // Check if snake is at max length
  if (snake_l == max_len) {
    // If yes, display win and restart
    byte win_scene[8] = { B11100011, B00100100, B01000010, B11100100, B00000011, 0, B00011100, 0 };
    for (int i = 1; i < 9; i++)
      SendData(i, win_scene[i - 1]);
    delay(5000);
    init_game();
  }

  // Check if snake has collided with itself
  for (int i = 0; i < max_len - 1; i++) {
    if (snake[i][0] == snake[max_len - 1][0] && snake[i][1] == snake[max_len - 1][1]) {
      // If yes, blink all leds and restart the game
      delay(1000);
      for (int j = 0; j < 4; j++) {
        SendData(DISPLAY_TEST, 0x01);
        delay(500);
        SendData(DISPLAY_TEST, 0x00);
        delay(500);
      }
      init_game();
      break;
    }
  }
  
  // Keep refreshing the matrix with updated data....
  refresh_scene();
  // ...Every 0.5 secs
  delay(500);
}