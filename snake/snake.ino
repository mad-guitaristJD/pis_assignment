#include <SPI.h>

#define CS 9


#define DECODE_MODE 9
#define INTENSITY 0x0A
#define SCAN_LIMIT 0x0B
#define SHUTDOWN 0x0C
#define DISPLAY_TEST 0x0F


#define left_button 2
#define right_button 3

volatile byte move_left = 0;
volatile byte move_right = 0;


int snake_l = 2;
const int max_len = 15;
int snake[max_len][2];
byte cur_heading = 0;


int blob[2] = { 0, 0 };
int is_eaten = 1;


byte scene[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


void SendData(uint8_t address, uint8_t value) {
  digitalWrite(CS, LOW);
  SPI.transfer(address);   
  SPI.transfer(value);     
  digitalWrite(CS, HIGH);  
}


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


void spawn_snake() {
  for (int j = max_len - snake_l; j < max_len; j++) {
    if (snake[j][0] <= 0)
      snake[j][0] = 8 + snake[j][0];
    else if (snake[j][0] >= 9)
      snake[j][0] = snake[j][0] - 8;
    if (snake[j][1] <= 0)
      snake[j][1] = 8 + snake[j][1];
    else if (snake[j][1] >= 9)
      snake[j][1] = snake[j][1] - 8;
    scene[snake[j][0] - 1] |= (1 << (snake[j][1] - 1));
  }
}


void snake_move() {
  if (snake[max_len - 1][0] == blob[0] && snake[max_len - 1][1] == blob[1]) {
    is_eaten = 1;
    snake_l += 1;
  }
  for (int i = snake_l - 1; i >= 1; i--) {
    snake[max_len - 1 - i][0] = snake[max_len - i][0];
    snake[max_len - 1 - i][1] = snake[max_len - i][1];
  }
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

void blob_generator() {
  if (is_eaten) {
    blob[0] = random(1, 9);
    blob[1] = random(1, 9);
  }
  scene[blob[0] - 1] |= (1 << (blob[1] - 1));
  is_eaten = 0;
}

void refresh_scene() {
  for (int i = 0; i < 8; i++)
    scene[i] = 0x00;
  snake_move();
  spawn_snake();
  blob_generator();
  byte rotated_scene[8];
rotate_scene_90_clockwise(scene, rotated_scene);
for (int i = 1; i < 9; i++)
  SendData(i, rotated_scene[i - 1]);

}

void update_left() {
  move_left = 1;
}

void update_right() {
  move_right = 1;
}

void rotate_scene_90_clockwise(byte input[8], byte output[8]) {
  for (int i = 0; i < 8; i++) {
    output[i] = 0;
    for (int j = 0; j < 8; j++) {
      if (input[7 - j] & (1 << i)) {
        output[i] |= (1 << j);
      }
    }
  }
}


void setup() {
  pinMode(left_button, INPUT_PULLUP);
  pinMode(right_button, INPUT_PULLUP);
  pinMode(CS, OUTPUT);
  
  SPI.setBitOrder(MSBFIRST);     
  SPI.begin();                   
  SendData(DISPLAY_TEST, 0x00);  
  SendData(DECODE_MODE, 0x00);   
  SendData(INTENSITY, 0x01);    
  SendData(SCAN_LIMIT, 0x0f);    
  SendData(SHUTDOWN, 0x01);      


  randomSeed(analogRead(0));

  attachInterrupt(digitalPinToInterrupt(left_button), update_left, FALLING);
  attachInterrupt(digitalPinToInterrupt(right_button), update_right, FALLING);

  sei();

  init_game();
}

void loop() {
  if (snake_l == max_len) {
    byte win_scene[8] = { B11100011, B00100100, B01000010, B11100100, B00000011, 0, B00011100, 0 };
    for (int i = 1; i < 9; i++)
      SendData(i, win_scene[i - 1]);
    delay(5000);
    init_game();
  }

  for (int i = 0; i < max_len - 1; i++) {
    if (snake[i][0] == snake[max_len - 1][0] && snake[i][1] == snake[max_len - 1][1]) {
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
  
  refresh_scene();
  delay(500);
}