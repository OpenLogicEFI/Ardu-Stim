/* vim: set syntax=c expandtab sw=2 softtabstop=2 autoindent smartindent smarttab : */
/*
 * Arbritrary wheel pattern generator
 *
 * copyright 2014 David J. Andruczyk
 * 
 * Ardu-Stim software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ArduStim software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with any ArduStim software.  If not, see http://www.gnu.org/licenses/
 *
 */

#include "globals.h"
#include "ardustim.h"
#include "enums.h"
#include "comms.h"
#include "storage.h"
#include "wheel_defs.h"
#include "platform_compat.h"
#include <math.h>
#if defined(__AVR__)
#include <util/delay.h>
#endif

/* External Globla Variables */
extern wheels Wheels[];

/* Volatile variables (USED in ISR's) */
extern volatile bool normal;
extern volatile uint16_t edge_counter;
extern volatile uint16_t new_OCR1A;
#if defined(ARDUSTIM_F401) || defined(ARDUSTIM_ESP32C3)
extern volatile uint32_t cycleDuration;
extern volatile uint32_t cycleStartTime;
#endif

bool cmdPending;
byte currentCommand;

//! Initializes the serial port and sets up the Menu
/*!
 * Sets up the serial port and menu for the serial user interface
 * Sets user input timeout to 20 seconds and overall interactivity timeout at 30
 * at which point it'll disconnect the user
 */
void serialSetup()
{
  Serial.begin(115200);
  cmdPending = false;
}

void commandParser()
{
  char buf[80];
  byte tmp_wheel;
  void* pnt_Config = &config;
  if (cmdPending == false) { currentCommand = Serial.read(); }

  switch (currentCommand)
  {
    case 'a':
      break;

    case 'c': //Receive a full config buffer
    {
      configTable receivedConfig = config;
      while(Serial.available() < int(sizeof(configTable) - 1)) {} // Version is not sent
      for(uint8_t x=1; x<(sizeof(struct configTable)); x++)
      {
        *((uint8_t *)&receivedConfig + x) = Serial.read();
      }
      if (receivedConfig.wheel >= MAX_WHEELS || receivedConfig.mode >= MAX_MODES) { break; }
      const bool wheelChanged = receivedConfig.wheel != config.wheel;
      // Publish a complete packet: the timer must never see a partial wheel/config.
      noInterrupts();
      config = receivedConfig;
      interrupts();
      if (wheelChanged) { display_new_wheel(); }
      break;
    }

    case 'C': //Send the current config
      for(uint8_t x=0; x<sizeof(struct configTable); x++)
      {
        Serial.write(*((uint8_t *)pnt_Config + x)); //Each byte is simply the location in memory of the config Page + the offset
      }
      break;
      
    case 'L': // send the list of wheel names
      //First byte sent is the number of wheels
      //Serial.println(MAX_WHEELS);
      
      //Wheel names are then sent 1 per line
      for(byte x=0;x<MAX_WHEELS;x++)
      {
        strcpy_P(buf,Wheels[x].decoder_name);
        Serial.println(buf);
      }
      break;

    case 'n': //Send the number of wheels
      Serial.println(MAX_WHEELS);
      break;

    case 'N': //Send the number of the current wheel
      Serial.println(config.wheel);
      break;
    
    case 'p': //Send the size of the current wheel
      Serial.println(Wheels[config.wheel].wheel_max_edges);
      break;

    case 'P': //Send the pattern for the current wheel
      for(uint16_t x=0; x<Wheels[config.wheel].wheel_max_edges; x++)
      {
        if(x != 0) { Serial.print(","); }

        byte tempByte = pgm_read_byte(&Wheels[config.wheel].edge_states_ptr[x]);
        Serial.print(tempByte);
      }
      Serial.println("");
      //2nd row of data sent is the number of degrees the wheel runs over (360 or 720 typically)
      Serial.println(Wheels[config.wheel].wheel_degrees);
      break;

    case 'R': //Send the current RPM
      Serial.println(currentStatus.rpm);
      break;

    case 'r': //Set the high and low RPM for sweep mode
      config.mode = LINEAR_SWEPT_RPM;
      while(Serial.available() < 6) {} //Wait for 4 bytes representing the new low and high RPMs

      // Read explicitly in wire order; C++ argument evaluation order varies by CPU.
      for (uint8_t i = 0; i < 3; ++i)
      {
        const uint8_t high = Serial.read();
        const uint8_t low = Serial.read();
        const uint16_t value = word(high, low);
        if (i == 0) { config.sweep_low_rpm = value; }
        else if (i == 1) { config.sweep_high_rpm = value; }
        else { config.sweep_interval = value; }
      }

      //sweep_low_rpm = 100;
      //sweep_high_rpm = 4000;
      break;

    case 's': //Save the current config
      saveConfig();
      break;

    case 'S': //Set the current wheel
      while(Serial.available() < 1) {} 
      tmp_wheel = Serial.read();
      if(tmp_wheel < MAX_WHEELS)
      {
        config.wheel = tmp_wheel;
        display_new_wheel();
      }
      break;

    case 'X': //Just a test method for switching the to the next wheel
      select_next_wheel_cb();
      strcpy_P(buf,Wheels[config.wheel].decoder_name);
      Serial.println(buf);
      break;

    default:
      break;
  }
  cmdPending = false;
}

/* Helper function to spit out amount of ram remainig */
//! Returns the amount of freeRAM
/*!
 * Figures out the amount of free RAM remaining nad returns it to the caller
 * \return amount of free memory
 */
#if defined(__AVR__)
uint16_t freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

#endif

/* SerialUI Callbacks */
//! Inverts the polarity of the primary output signal
void toggle_invert_primary_cb()
{
  extern uint8_t output_invert_mask;
  output_invert_mask ^= 0x01; /* Flip crank invert mask bit */

}

//! Inverts the polarity of the secondary output signal
void toggle_invert_secondary_cb()
{
  extern uint8_t output_invert_mask;
  output_invert_mask ^= 0x02; /* Flip cam invert mask bit */
}

void display_new_wheel()
{
#if defined(ARDUSTIM_F401) || defined(ARDUSTIM_ESP32C3)
  noInterrupts();
#endif
  reset_new_OCR1A(currentStatus.rpm);
  edge_counter = 0; // Reset to beginning of the wheel pattern */
#if defined(ARDUSTIM_F401) || defined(ARDUSTIM_ESP32C3)
  cycleDuration = 0;
  cycleStartTime = micros();
  interrupts();
#endif
}


//! Selects the next wheel in the list
/*!
 * Selects the next wheel, if at the end, wrap to the beginning of the list,
 * re-calculate the OCR1A value (RPM) and reset, return user information on the
 * selected wheel and current RPM
 */
void select_next_wheel_cb()
{
  if (config.wheel == (MAX_WHEELS-1))
    config.wheel = 0;
  else 
    config.wheel++;
  
  display_new_wheel();
}

//
//! Selects the previous wheel in the list
/*!
 * Selects the nex, if at the beginning, wrap to the end of the list,
 * re-calculate the OCR1A value (RPM) and reset, return user information on the
 * selected wheel and current RPM
 */
void select_previous_wheel_cb()
{
  if (config.wheel == 0)
    config.wheel = MAX_WHEELS-1;
  else 
    config.wheel--;
  
  display_new_wheel();
}
