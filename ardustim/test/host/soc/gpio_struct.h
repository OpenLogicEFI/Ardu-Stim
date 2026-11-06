#pragma once
#include <stdint.h>
struct TestGPIORegister { uint32_t val = 0; };
struct TestGPIO { TestGPIORegister out_w1tc, out_w1ts; };
static TestGPIO GPIO;
