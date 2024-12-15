#include <FreqCount.h>
#include "options.h"
#include <EncButton.h>
Button btn(BTN_PIN, INPUT_PULLUP);

#include <GyverSegment.h>
Disp1637Colon disp(DIO_PIN, CLK_PIN);

#include <GyverFilters.h>
GMedian<6, int32_t> diffFilter;
GKalman diffFilterKalman(6, 6, 0.25);

#include <EEManager.h>  // подключаем либу
EEManager memory(data); // передаём нашу переменную (фактически её адрес)

#include "service.h"


void setup() {
  Serial.begin(115200);
  memory.begin(0, 'b');
  disp.brightness(4);
  deviceInit();
  dspPrint("StAr");

  //Read out baseline frequency count, 100ms intervals
  FreqCount.begin(100);
  while (!FreqCount.available()) {
    delay(10);
  }

  baseLine = FreqCount.read();

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
  delay(1000);
  displayMode = DIFF;
}

void loop() {
  btn.tick();
  buzzerHandle();
  disp.tick();
  memory.tick();
  buzzerTrashHoldHandle();
  btnHandle();
  dspHandle();
  
  //no sample ready yet, exit.
  if (!FreqCount.available()) return;

  //Read how many pulses in 100 milliseconds
  readGenerator();

  // Check Generator
  checkWorkGenerator();
  
//  serialInfo();
  
  //Auto-adjust our baseline
  autoAdjust();
}
