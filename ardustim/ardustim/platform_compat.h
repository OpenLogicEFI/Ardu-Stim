#ifndef ARDUSTIM_PLATFORM_COMPAT_H
#define ARDUSTIM_PLATFORM_COMPAT_H

#include <Arduino.h>
#if defined(ARDUSTIM_F401)
#if !defined(STM32F401xC)
#error "ARDUSTIM_F401 requires the STM32F401CC Black Pill target"
#endif
#include <avr/pgmspace.h> // STM32duino supplies the Arduino flash-access compatibility API
#elif defined(__AVR__)
#include <avr/pgmspace.h>
#else
#error "Unsupported Ardu-Stim platform"
#endif

#endif
