#include <FreqCounter.h>
#include "options.h"

#include <EncButton.h>
Button btn(BTN_PIN, INPUT_PULLUP);

#include <GyverSegment.h>
Disp1637Colon disp(DIO_PIN, CLK_PIN);

#include <GyverFilters.h>
GMedian<10, int32_t> diffFilter;
GKalman diffFilterKalman(6, 6, 0.35);

#include <EEManager.h>  // подключаем либу
EEManager memory(data); // передаём нашу переменную (фактически её адрес)

#include "service.h"


void setup() {
  Serial.begin(9600);
  memory.begin(0, 'a');
  disp.brightness(4);
  deviceInit();
  dspPrint("StaR");
  delay(2000);
  initBaseLine();
  
  

  if (baseLine > 10000 ) {
    dspPrint("RUNG");
    Serial.println("Generator active");
  } else {
    dspPrint("ErrG");
    Serial.print("Generator base line: ");
    Serial.print(baseLine);
    Serial.println(".");
    Serial.println("Generator error");
  }
  delay(200);
  displayMode = DIFF;
}

void loop() {
  disp.tick();
  
  btn.tick();
  buzzerHandle();
  batteryControl();
  memory.tick();
  buzzerTrashHoldHandle();
  btnHandle();
  dspHandle();
  batteryHandle();
  
  // wait if any serial is going on
  FreqCounter::f_comp=10;   // Cal Value / Calibrate with professional Freq Counter
  FreqCounter::start(100);  // 100 ms Gate Time
  //no sample ready yet, exit.
  while (FreqCounter::f_ready == 0) ;
  
  //Read how many pulses in 100 milliseconds
  readGenerator();
  serialInfo();
  // Check Generator
  checkWorkGenerator();
  
//  serialInfo();
  
  //Auto-adjust our baseline
  autoAdjust();
}
