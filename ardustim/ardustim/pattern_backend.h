#pragma once
#if defined(ARDUSTIM_F401)
#include "f401_backend.h"
#define backendSetup f401Setup
#define backendStart f401Start
#define backendStop f401Stop
#define backendMicros f401Micros
#define backendSetRPM f401SetRPM
#define backendWriteOutputs f401WriteOutputs
#define backendPollADC f401PollADC
#define backendBeginSave f401BeginSave
#define backendEndSave f401EndSave
#elif defined(ARDUSTIM_ESP32C3)
#include <stdint.h>
void patternTick();
void backendSetup();
void backendStart();
void backendStop();
uint32_t backendMicros();
void backendSetRPM(float scaler, uint32_t rpm);
void backendWriteOutputs(uint8_t state);
void backendPollADC();
void backendBeginSave();
void backendEndSave();
#endif
