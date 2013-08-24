/*
 Copyright (C) 2011 James Coliz, Jr. <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example of a sensor network 
 *
 * This sketch demonstrates how to use the RF24Network library to
 * manage a set of low-power sensor nodes which mostly sleep but
 * awake regularly to send readings to the base.
 *
 * The example uses TWO sensors, a 'temperature' sensor and a 'voltage'
 * sensor.
 *
 * To see the underlying frames being relayed, compile RF24Network with
 * #define SERIAL_DEBUG.
 *
 * The logical node address of each node is set in EEPROM.  The nodeconfig
 * module handles this by listening for a digit (0-9) on the serial port,
 * and writing that number to EEPROM.
 */

#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Tictocs.h>
#include <Button.h>
#include <TictocTimer.h>
#include "nodeconfig.h"
#include "sleep.h"
#include "S_message.h"
#include "printf.h"

// This is for git version tracking.  Safe to ignore
#ifdef VERSION_H
#include "version.h"
#else
const char program_version[] = "Unknown";
#endif

// Pin definitions
#ifndef PINS_DEFINED
#define __PLATFORM__ "Getting Started board"

// Pins for radio
const int rf_ce = 9;
const int rf_csn = 10;
#endif

RF24 radio(rf_ce,rf_csn);
RF24Network network(radio);

// Our node configuration 
eeprom_info_t this_node;

/**
 * Convenience class for handling LEDs.  Handles the case where the
 * LED may not be populated on the board, so always checks whether
 * the pin is valid before setting a value.
 */

// Nodes in test mode do not sleep, but instead constantly try to send
bool test_mode = false;

void setup(void)
{
  //
  // Print preamble
  //
  
  Serial.begin(57600);
  printf_begin();
  printf_P(PSTR("\n\rRF24Network/examples/sensornet/\n\r"));
  printf_P(PSTR("PLATFORM: " __PLATFORM__ "\n\r"),program_version);
  printf_P(PSTR("VERSION: %s\n\r"),program_version);

  // Which node are we?
  this_node = nodeconfig_read();

  //
  // Bring up the RF network
  //
  SPI.begin();
  radio.begin();
  network.begin(/*channel*/ 92, /*node address*/ this_node.address);
}

void loop(void)
{
  // Pump the network regularly
  network.update();

  // If we are the base, is there anything ready for us?
  // This one is for base, what are you gonna do with base
  while ( network.available() )
  {
    // If so, grab it and print it out
    RF24NetworkHeader header;
    S_message message;
    network.read(header,&message,sizeof(message));
    printf_P(PSTR("%lu: APP Received #%u %s from 0%o\n\r"),millis(),header.id,message.toString(),header.from_node);
    delay(1000);
  }

  // If we are the kind of node that sends readings, AND it's time to send
  // a reading AND we're in the mode where we send readings...
  // This one is for Leaf or Nodes action either you want to send data or receive data with something
  if ( this_node.address > 0 )
  {
    int i;
    S_message message;
    
    printf_P(PSTR("---------------------------------\n\r"));
    printf_P(PSTR("%lu: APP Sending %s to 0%o...\n\r"),millis(),message.toString(),0);
    
    // Send it to the base
    RF24NetworkHeader header(/*to node*/ 0, /*type*/ test_mode ? 's' : 'S');
    bool ok = network.write(header,&message,sizeof(message));
    if (ok)
    {
      printf_P(PSTR("%lu: APP Send ok\n\r"),millis());
    }
    else
    {
      printf_P(PSTR("%lu: APP Send failed\n\r"),millis());
    }
  }

  // Listen for a new node address
  nodeconfig_listen();
}
// vim:ai:cin:sts=2 sw=2 ft=cpp
