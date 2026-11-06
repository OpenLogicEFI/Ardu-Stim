#ifndef ARDUSTIM_F401_BACKEND_H
#define ARDUSTIM_F401_BACKEND_H

#include <stdint.h>

void patternTick();
void f401Setup();
void f401Start();
void f401Stop();
uint32_t f401Micros();
void f401SetRPM(float scaler, uint32_t rpm);
void f401WriteOutputs(uint8_t state);
void f401PollADC();
void f401BeginSave();
void f401EndSave();

#endif
