#pragma once

#include <Arduino.h>

#define DIO_PIN       7
#define CLK_PIN       8
#define POT_SENS_PIN  A1 

#define BTN_PIN       3
#define BUZZER_PIN    12

#define SENSI 100

#define TRASH_HOLD_MAX 26
#define TIMEOUT_SHOW_DELAY 3000

#define TACT(x) x

enum {
  DIFF,
  TRASHHOLD,
  ERROR_GENERATOR,
  FREQ
  
} displayMode;

struct Data {
  bool useBuzzer = true;
  bool useFilter = false;
  bool useFilterKalman = false;  
};

Data data;

//the baseline frequency of the main search coil
uint32_t  baseLine = 0;

uint8_t trashHold = 3;
uint32_t count = 0;
int32_t difference = 0;
int32_t diffFiltVal = 0;
int32_t diffFiltKalmanVal = 0;
