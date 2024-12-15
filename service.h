#pragma once

void deviceInit()
{
  pinMode(POT_SENS_PIN, INPUT);
  
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW); // D9 Pin GND
  
  pinMode(BUZZER_PIN, OUTPUT);
}

void serialInfo()
{
  static uint16_t t;
  uint16_t cm = millis();
  if ( cm - t >= 100 ) {
    t = cm;
    Serial.print("Freq: ");
    Serial.println(count);
    Serial.print("TrashHold Buzzer: ");
    Serial.println( trashHold );
    Serial.print("Base Line: ");
    Serial.println(baseLine);
    Serial.print("Diff: ");
    Serial.println(difference);
    Serial.print("Abs  Diff: ");
    Serial.println( abs(difference) );
  }
}

uint8_t getTrashHoldBuzzer()
{
  return (uint8_t) map(analogRead(POT_SENS_PIN), 0, 1023, 1, TRASH_HOLD_MAX );
}

void buzzerTrashHoldHandle()
{
  static uint8_t t;
  uint8_t cm = millis();
  if ( (uint8_t)(cm - t) >= 64 ) {
    t = cm;
    trashHold = getTrashHoldBuzzer();
  }
}

void buzzerHandle()
{
  if (!data.useBuzzer) return;
  if (trashHold == TRASH_HOLD_MAX) {
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }
  digitalWrite(BUZZER_PIN, (difference > trashHold) ? HIGH : LOW);
}

void autoAdjust() 
{
  if (count > baseLine) {
    baseLine++;
  } else if (count < baseLine) {
    baseLine--;
  }
}



void readGenerator()
{
  count = FreqCount.read();
  difference = baseLine - count;
  difference = abs(difference);
  diffFiltVal = diffFilter.filtered(difference);
  diffFiltKalmanVal = diffFilterKalman.filtered(difference);
  if (data.useFilter) {
    difference = (data.useFilterKalman) ? diffFiltKalmanVal : diffFiltVal ;
  }
}

// ============== Display =============================

void dspPrint(String s)
{
  disp.clear();
  disp.setCursor(0);
  disp.print(s.c_str());
  disp.update();
  disp.delay(TIMEOUT_SHOW_DELAY);
}

void dspErrorGenerator()
{
  dspPrint("ERRG");
}

void dspShowSrart()
{
  dspPrint("StAr");
}

void dspShowDiff()
{
  disp.clear();
  disp.setCursorEnd();
  disp.printRight(true);
  disp.fillChar('_');
  disp.print(difference);
  disp.update();
}

void dspShowFreq()
{
  disp.clear();
  disp.setCursorEnd();
  disp.printRight(true);
  disp.fillChar('_');
  disp.print( (count / 10) );
  disp.update();
  disp.delay(TIMEOUT_SHOW_DELAY);
}

void dspShowTrashHold()
{
  disp.clear();
  disp.setCursorEnd();
  disp.printRight(true);
  disp.fillChar('-');
  if ( trashHold < TRASH_HOLD_MAX ) {
    disp.print(trashHold);
  } else {
    disp.print("BOFF");
  }
  disp.update();
  disp.delay(TIMEOUT_SHOW_DELAY);
}

void dspLevelInfo()
{
  disp.clear();
  uint16_t trH = trashHold * 5;
  uint8_t lw = map(constrain(difference, 0, trH), 0, trH, 0, 3);
  switch (lw) {
    case 0:
      disp.setCursor(3);
      disp.print("_");
      disp.update();
      break;
    case 1:
      disp.setCursor(3);
      disp.print("0");
      disp.update();
      break;
    case 2:
      disp.setCursor(2);
      disp.print("00");
      disp.update();
      break;
    case 3:
      disp.setCursor(1);
      disp.print("000");
      disp.update();
      break;
  }
  disp.delay(100);
}

void dspHandle()
{
  switch(displayMode) {
    case DIFF: 
     if (data.useBuzzer) dspShowDiff();
     else dspLevelInfo();
    break;
    case TRASHHOLD: dspShowTrashHold(); break;
    case ERROR_GENERATOR: dspErrorGenerator(); break;
    case FREQ: dspShowFreq(); break;
  }
}

// ============== END Display =============================

void btnHandle()
{
  if (btn.hold()) {
    displayMode = DIFF;
    data.useFilter = false;
    data.useFilterKalman = false;
    memory.update();
    dspPrint("DEFF");
  }

  if (btn.step(1)) {
    dspPrint("BS_0");
    baseLine = count;
  }
  
  if (btn.hasClicks()) {
    switch(btn.getClicks()) {
      case 1 :
        data.useBuzzer = !data.useBuzzer;
        dspPrint( (data.useBuzzer) ? "BUZZ" : "DSPL" );
        memory.update();
        break;
      case 2:
        displayMode = TRASHHOLD;
        break;
      case 3:
        displayMode = FREQ;
        break;
      case 4:
        data.useFilter = true;
        dspPrint( (data.useFilter) ? "FILT" : "RAW" );
        memory.update();
        break;
      case 5:
        data.useFilterKalman = true;
        dspPrint( (data.useFilterKalman) ? "FILK" : "RAWM" );
        memory.update();
        break;
    }
    
  }
}

void checkWorkGenerator()
{
  if (count == 0) {
    disp.point(false);
    displayMode = ERROR_GENERATOR;
  } else {
    disp.point(true);
    displayMode = DIFF;
  }
}
