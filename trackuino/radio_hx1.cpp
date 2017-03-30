/* trackuino copyright (C) 2010  EA5HAV Javi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "config.h"
#include "pin.h"
#include "radio_hx1.h"
#if (ARDUINO + 1) >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif


void RadioHx1::setup()
{
  // Configure pins
  pinMode(PTT_PIN, OUTPUT);
#ifdef POWER_MODE
  pinMode(POWER_PIN, OUTPUT);
#endif
  
#if PTT_INV == 0
  pin_write(PTT_PIN, LOW);
#else
  pin_write(PTT_PIN, HIGH);
#endif
  pinMode(AUDIO_PIN, OUTPUT);
}

void RadioHx1::ptt_on()
{
#if PTT_INV == 0
  pin_write(PTT_PIN, HIGH);
#else
  pin_write(PTT_PIN, LOW);
#endif

#if POWER_MODE == 2
  pin_write(POWER_PIN, HIGH); //High power select
#elif POWER_MODE == 1
  pin_write(POWER_PIN, LOW); //Low power select
#endif

  delay(25);   // The HX1 takes 5 ms from PTT to full RF, give it 25
}

void RadioHx1::ptt_off()
{
  pin_write(PTT_PIN, HIGH);
#ifdef POWER_MODE
  pin_write(POWER_PIN, LOW); //Low power
#endif
}
