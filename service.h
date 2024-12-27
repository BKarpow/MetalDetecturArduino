#pragma once

long readVcc()
{
  // Вимірюємо внутрішнє опорне джерело 1.1 В
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); // Вибір внутрішнього 1.1 В як входу АЦП
  delay(2); // Затримка для стабілізації
  ADCSRA |= _BV(ADSC); // Запускаємо конверсію АЦП
  while (bit_is_set(ADCSRA, ADSC)); // Чекаємо завершення

  uint8_t low = ADCL;  // Зчитуємо молодший байт
  uint8_t high = ADCH; // Зчитуємо старший байт
  long result = (high << 8) | low; // Об'єднуємо обидва байти

  // Обчислюємо VCC за формулою
  result = 1125300L / result; // 1.1 В * 1023 * 1000 / результат вимірювання
  return result; // Повертаємо VCC у мілівольтах
}

uint16_t getBatteryMilivolt()
{
  uint16_t bRaw = analogRead(BATTERY_PIN);
  return constrain( (uint16_t)(bRaw * ( 5000 / 1023)), 2700, 4200);
}

void batteryHandle()
{
  static uint16_t t;
  uint16_t cm = millis();
  if ( cm - t > 1024 ) {
    t = cm;
    batteryLevel = map(getBatteryMilivolt(), 2700, 4200, 1, 100);
  }
}

void batteryControl()
{
  isLowLevelBattery = batteryLevel < 10;
}

void deviceInit()
{
  pinMode(POT_SENS_PIN, INPUT);
  pinMode(BATTERY_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // For buzzer GND
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
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
  if (!data.useBuzzer) {
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }
  if (trashHold == TRASH_HOLD_MAX) {
    digitalWrite(BUZZER_PIN, LOW);
    return;
  }
  digitalWrite(BUZZER_PIN, (difference > 2) ? HIGH : LOW);
}

void autoAdjust() 
{
  if (count > baseLine) {
    baseLine++;
  } else if (count < baseLine) {
    baseLine--;
  }
}

void initBaseLine() 
{
  for(uint8_t i = 0; i < 10 ; i++) {
    // wait if any serial is going on
    FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
    FreqCounter::start(100);  // 100 ms Gate Time

    while (FreqCounter::f_ready == 0) ;
    baseLine = FreqCounter::f_freq;
  }
}

void readGenerator()
{
  count = FreqCounter::f_freq;
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
  disp.setCursorEnd();
  disp.printRight(true);
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

void dspShowVcc()
{
  
  disp.clear();
  disp.setCursorEnd();
  disp.printRight(true);
  disp.fillChar('b');
  disp.print(batteryLevel);
  disp.update();
  disp.delay(TIMEOUT_SHOW_DELAY);
}

void dspShowDiff()
{
  disp.clear();
  disp.setCursorEnd();
  disp.printRight(true);
  disp.fillChar( (isLowLevelBattery) ? "L" : "_");
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
     dspShowDiff();
    break;
    case TRASHHOLD: dspShowTrashHold(); break;
    case ERROR_GENERATOR: dspErrorGenerator(); break;
    case FREQ: dspShowFreq(); break;
    case VCC: dspShowVcc(); break;
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
  if (btn.step(2)) displayMode = FREQ;
  
  if (btn.hasClicks()) {
    switch(btn.getClicks()) {
      case 1 :
        data.useBuzzer = !data.useBuzzer;
        dspPrint( (data.useBuzzer) ? "BUZZ" : "DSPL" );
        memory.update();
        break;
      case 2:
        displayMode = FREQ;
        break;
      case 3:
        displayMode = VCC;
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
