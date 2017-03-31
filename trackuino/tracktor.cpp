/* tracktor copyright (C) 2017, LZ1ARM
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
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include "pin.h"
#include "config.h"
#include "tracktor.h"

#define GPS_RESET_PIN    17
#define RF_PD_PIN        18
#define RF_H_L_PIN       19
#define BT_ENABLE_PIN    4
#define GPS_EN_PIN       5

void gps_reset_on()
{
	pin_write(GPS_EN_PIN, LOW);
}

void gps_reset_off()
{
	pin_write(GPS_EN_PIN, HIGH);
}

void gps_reset()
{
    gps_reset_on();
    delay(50);
    gps_reset_off();
    delay(50);
}

void gps_standby()
{
    Serial.println("$PMTK161,0*28");
    delay(500L);
}

void gps_wakeup()
{
    Serial.println("$PMTK104*37\r\n");
    delay(500);
}

void radio_setup()
{
    delay(2000);
	Serial.println("AT+DMOSETGROUP=1,144.8000,144.8000,0000,0,0000");
    delay(500L);
    Serial.println("AT+DMOSETVOLUME=3");
    delay(500L);
}

void board_reset()
{
    gps_reset();
}

void board_gpio_setup()
{
    pinMode(GPS_RESET_PIN, OUTPUT);
    pinMode(GPS_EN_PIN, OUTPUT);
    pinMode(RF_PD_PIN, OUTPUT);
    pinMode(RF_PD_PIN, OUTPUT);
    pinMode(RF_H_L_PIN, OUTPUT);
    pinMode(BT_ENABLE_PIN, OUTPUT);
    pinMode(PTT_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    pin_write(PTT_PIN, HIGH);      // DPTT RX
    pin_write(RF_PD_PIN, HIGH);    // RF Power enable
    pin_write(RF_H_L_PIN, LOW);    // RF low power
    pin_write(GPS_RESET_PIN, HIGH); // RF low power
    pin_write(GPS_EN_PIN, LOW);    // GPS Enable low
    pin_write(GPS_EN_PIN, LOW);    // GPS Enable low
    pin_write(BT_ENABLE_PIN, LOW); // Bluetooth disable
    pin_write(LED_PIN, HIGH);      // LED ON
}

