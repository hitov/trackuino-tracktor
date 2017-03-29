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

// Mpide 22 fails to compile Arduino code because it stupidly defines ARDUINO 
// as an empty macro (hence the +0 hack). UNO32 builds are fine. Just use the
// real Arduino IDE for Arduino builds. Optionally complain to the Mpide
// authors to fix the broken macro.
#if (ARDUINO + 0) == 0
#error "Oops! We need the real Arduino IDE (version 22 or 23) for Arduino builds."
#error "See trackuino.pde for details on this"

// Refuse to compile on arduino version 21 or lower. 22 includes an 
// optimization of the USART code that is critical for real-time operation
// of the AVR code.
#elif (ARDUINO + 0) < 22
#error "Oops! We need Arduino 22 or 23"
#error "See trackuino.pde for details on this"

#endif


// Trackuino custom libs
#include "config.h"
#include "afsk_avr.h"
#include "afsk_pic32.h"
#include "aprs.h"
#include "buzzer.h"
#include "gps.h"
#include "pin.h"
#include "power.h"
#include "sensors_avr.h"
#include "sensors_pic32.h"

// Arduino/AVR libs
#if (ARDUINO + 1) >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif

// Module constants
static const uint32_t VALID_POS_TIMEOUT = 2000;  // ms

// Module variables
static int32_t next_aprs = 0;


void setup()
{

    DDRC |= 1 << 2; // DPTT output
    PORTC |= 1 << 2; // DPTT RX
    DDRC |= 1 << 4; //PD output
    PORTC |= 1 << 4; //PD off
    DDRC |= 1 << 5; //High/Low output
    PORTC &= ~(1 << 5); //Low power
    //PORTC |= (1 << 5); //HIGH power

    DDRD |= 1 << 7; // LED output
    PORTD |= 1 << 7; // LED RX

    DDRC |= 1 << 3; // GPS RESET output
    PORTC &= ~(1 << 3); // GPS RESET active
    PORTC |= (1 << 3); // GPS RESET active

    DDRD |= 1 << 5; //High/Low output
    PORTD &= ~(1 << 5); //Low power

    DDRD |= 1 << 4; //Bluetooth enble
    PORTD &= ~(1 << 4); //Bluetooth disable
  
  pinMode(LED_PIN, OUTPUT);
  pin_write(LED_PIN, HIGH);

  delay(1000);
  Serial.begin(9600);


      PORTC &= ~(1 << 3); // GPS RESET active
    delay(100);
    PORTC |= (1 << 3); // GPS RESET active
   
  delay(2000);
  
  Serial.println("$PMTK161,0*28");
  delay(500L);
  Serial.println("AT+DMOSETGROUP=1,144.8000,144.8000,0000,0,0000");
  delay(500L);
  Serial.println("AT+DMOSETVOLUME=3");
  delay(500L);
  
  //Serial.println("AT+DMOSETGROUP=0,144.8000,144.8000,1,2,1,1");
  //Serial.println("$PMTK183*38");
  //Serial.println("$PMTK104*37\r\n");
  Serial.println("$PMTK104*37\r\n");
  //Serial.println("$PMTK104*37\r\n");
  delay(500);
//
//  DDRD |= 1<<6;
//  PORTD |= 1<<6;
//  TCCR0A = (1<<WGM01);
//  OCR0A = 127;
//  TCCR0B = (1 << CS01) | (1 << CS00);
//  TCCR0A = _BV(COM0A1) | _BV(WGM01) | _BV(WGM00);
//  TCCR0B = _BV(CS00);

  Serial.begin(GPS_BAUDRATE);
#ifdef DEBUG_RESET
  Serial.println("RESET");
#endif

  buzzer_setup();
  afsk_setup();
  gps_setup();
  sensors_setup();

#ifdef DEBUG_SENS
  Serial.print("Ti=");
  Serial.print(sensors_int_lm60());
  Serial.print(", Te=");
  Serial.print(sensors_ext_lm60());
  Serial.print(", Vin=");
  Serial.println(sensors_vin());
#endif
  // Do not start until we get a valid time reference
  // for slotted transmissions.
  if (APRS_SLOT >= 0) {
    do {
      while (! Serial.available());
        power_save();
    } while (! gps_decode(Serial.read()));
    
    next_aprs = millis() + 1000 *
      (APRS_PERIOD - (gps_seconds + APRS_PERIOD - APRS_SLOT) % APRS_PERIOD);
  }
  else {
    next_aprs = millis();
  }  
  // TODO: beep while we get a fix, maybe indicating the number of
  // visible satellites by a series of short beeps?
}

void get_pos()
{
  // Get a valid position from the GPS
  int valid_pos = 0;
  uint32_t timeout = millis();

#ifdef DEBUG_GPS
  Serial.println("\nget_pos()");
#endif

  gps_reset_parser();

  do {
    if (Serial.available())
      valid_pos = gps_decode(Serial.read());
  } while ( (millis() - timeout < VALID_POS_TIMEOUT) && ! valid_pos) ;

  if (valid_pos) {
    if (gps_altitude > BUZZER_ALTITUDE) {
      buzzer_off();   // In space, no one can hear you buzz
    } else {
      buzzer_on();
    }
  }
}

void loop()
{
  // Time for another APRS frame
  if ((int32_t) (millis() - next_aprs) >= 0) {
    get_pos();
    aprs_send();
    next_aprs += APRS_PERIOD * 1000L;
    while (afsk_flush()) {
      power_save();
    }

#ifdef DEBUG_MODEM
    // Show modem ISR stats from the previous transmission
    afsk_debug();
#endif

  } else {
    // Discard GPS data received during sleep window
    while (Serial.available()) {
      Serial.read();
    }
  }

  power_save(); // Incoming GPS data or interrupts will wake us up
}
