#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <Servo.h>

#define SERVO_SIGNAL A1
#define UNLOCK_SW A2
#define LOCK_SW A3

#define UNLOCK_PHRASE "unlock"
#define LOCK_PHRASE "lock"
#define STATE_PHRASE "state"

Servo servo;
bool isOpen;

void setup() {
  ble_begin();
  Serial.begin(57600);
  pinMode(INPUT, LOCK_SW);
  pinMode(INPUT, UNLOCK_SW);
  touchServo(0);
  isOpen = false;
}

void loop() {
  if (!ble_connected()) {
    watchSwitchEvent();
    ble_do_events();
    return;
  }
  
  if (ble_available()) {
    char command[255];
    readCommand(command);
    if (0==strcmp(command, UNLOCK_PHRASE)) {
      doUnlock();
    } else if (0==strcmp(command, LOCK_PHRASE)) {
      doLock();
    } else if (0==strcmp(command, STATE_PHRASE)) {
      sendStatus();
    }
  }
  
  watchSwitchEvent();
  
  ble_do_events();
}

void readCommand(char* chr) {
  int i = 0;
  chr[0] = '\0';
  while (ble_available()) {
    chr[i] = ble_read();
    i++;
  }
  chr[i] = '\0';
  Serial.println(chr);
}

void watchSwitchEvent() {
  if (digitalRead(LOCK_SW) == HIGH) {
    doLock();
  } else if (digitalRead(UNLOCK_SW) == HIGH) {
    doUnlock();
  }
}

void doLock() {
  touchServo(0);
  isOpen = false;
  sendStatus();
}

void doUnlock() {
  touchServo(90);
  isOpen = true;
  sendStatus();
}

void delay_with_ble(int n) {
  for (int i = 0; i < n/10; i++) {
    delay(10);
    ble_do_events();
  }
}

void touchServo(int degree) {
  servo.attach(SERVO_SIGNAL);
  delay_with_ble(100);
  servo.write(degree);
  // サーボが動き終わるまで適当な時間待つ。
  delay_with_ble(600);
  servo.detach();
  delay_with_ble(200);
}

void sendStatus() {
  if (isOpen) {
    byte buf[] = {'O', '\0'};
    ble_write_string(buf, 2);
  } else {
    byte buf[] = {'X', '\0'};
    ble_write_string(buf, 2);
  }
}

static byte buf_len = 0;
void ble_write_string(byte *bytes, uint8_t len) {
  if (buf_len + len > 20) {
    for (int j = 0; j < 15000; j++) {
      ble_do_events();
    }
    buf_len = 0;
  }
  
  for (int j = 0; j < len; j++) {
    ble_write(bytes[j]);
    buf_len++;
  }
    
  if (buf_len == 20) {
    for (int j = 0; j < 15000; j++) {
      ble_do_events();
    }
    buf_len = 0;
  }  
}

