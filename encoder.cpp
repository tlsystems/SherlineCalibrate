/*
   Copyright (C) 2009, 2010 Matt Reba, Jermeiah Dillingham

    This file is part of BrewTroller.

    BrewTroller is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BrewTroller is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BrewTroller.  If not, see <http://www.gnu.org/licenses/>.

  FermTroller - Open Source Fermentation Computer
  Software Lead: Matt Reba (matt_AT_brewtroller_DOT_com)
  Hardware Lead: Jeremiah Dillingham (jeremiah_AT_brewtroller_DOT_com)

  Documentation, Forums and more information available at http://www.brewtroller.com

  Compiled on Arduino-0017 (http://arduino.cc/en/Main/Software)
  With Sanguino Software v1.4 (http://code.google.com/p/sanguino/downloads/list)
  using PID Library v0.6 (Beta 6) (http://www.arduino.cc/playground/Code/PIDLibrary)
  using OneWire Library (http://www.arduino.cc/playground/Learning/OneWire)


  Original Author:  Matt Reba & Jason Vreeland (CodeRage)
  Modified By:      Tom Harkaway, Feb, 2011

  Modifications:

  1. Modified existing begin() method. 
    - Change order of parameter
    - Added an boolean "ActiveLow" parameter to specify if the encoder's switches
      are wired active-low (i.e. switch to ground). If it is active-low, the sense
      of the enter switch is reversed.
    - Require that the external interrupt number for both EncE and EncA be specified.

  2. Added a new begin() method that uses PinChange interrupts rather than External 
     interrupts for the EncE and EncA switches. Uses new PCInt functions added
     to FastPin library

  3. Modified Cancel logic so it triggers as soon as the cancel timeout had been reached
     rather than wait for enter to be released.

  4. General reorganization and additional comments.

***********************************************************/


#include "encoder.h"

// encoder object c'tor
//
encoder::encoder(void)
{
	_count = 0;
	_min = 0;
	_max = 0;
	_wrap = 0;
}


// initialize encoder
//  encE, encA, encB - pin numbers for enter, phaseA, and phaseB
//
//  Note: encE & encA must be interrupt pins
//
void encoder::begin(byte encE, byte encA, byte encB)
{
	_count = 0;
	
	pinMode(2, INPUT);
	pinMode(3, INPUT);
	pinMode(4, INPUT);
	
	_activeLow = false;
	
	// attach PinChange Interrupts
	noInterrupts();
	attachInterrupt(digitalPinToInterrupt(2), enterISR, RISING);
	attachInterrupt(digitalPinToInterrupt(3), phaseAISR, CHANGE);
	interrupts();
}


//Detaches the Encoder ISRs
void encoder::end(void)
{
	noInterrupts();
	detachInterrupt(digitalPinToInterrupt(_ePin));
	detachInterrupt(digitalPinToInterrupt(_aPin));
	interrupts();
}

// set activeLow state
//
void encoder::setActiveLow(bool state)
{
	_activeLow = state;
	if (_activeLow)
	{
		// turn on output to enable pull-ups
		digitalWrite(_aPin, HIGH);
		digitalWrite(_bPin, HIGH);
		digitalWrite(_ePin, HIGH);
	}
	else
	{
		digitalWrite(_aPin, LOW);
		digitalWrite(_bPin, LOW);
		digitalWrite(_ePin, LOW);
	}
}


// return value of encoder pins
//  bit-0 enter
//  bit-1 phase A
//  bit-2 phase B
//	bit-3 enter pin
//
byte  encoder::getEncoderState()
{
	byte btVal = 0;
	if (digitalRead(_ePin)) btVal |= 0x01;
	if (digitalRead(_aPin)) btVal |= 0x02;
	if (digitalRead(_bPin)) btVal |= 0x04;
	if (isEnterPinPressed()) btVal |= 0x08;
	btVal |= _enterState << 4;
	return btVal;
}



// encoder::getDelta()
//  - compares the current count to the last count
//  - updates last count to the current count
//  - returns the difference
//
int encoder::getDelta(void)
{
	int delta;
	int	count;

	count = getCount();

	delta = count - _lastCount;
	_lastCount = count;

	return delta;
}


// encoder::change()
//  If the count has not changed since the last time change was called
//    return -1
//  else 
//    update the last count and return the new count
//
int encoder::change(void)
{
	 return (getDelta()==0) ? -1 : _count;
}


// return ok state
//  if enterState == 1, reset enterState and return true
//
bool encoder::ok(void)
{
	bool okActive = (_enterState == 1);
	if (okActive)
		_enterState = 0;
	return okActive;
}


// return cancel state
//  if enterState == 2, reset enterState and return true
//
bool encoder::cancel(void)
{
	// check if cancel has already been detected and reported
	if (_enterState == 3)
		return false;
	
	noInterrupts();
	
	bool cancelState = (_enterState == 2);
	if (cancelState)
	{
		// enter ISR has detected cancel condition
		_enterState = 0;
	}
	else if (isEnterPinPressed() && isTimeElapsed(millis(), ENTER_LONG_PUSH))
	{
		// cancel condition detected
		cancelState = true;
		_enterState = 3;  // 3=cancel detected prior to release (used by ISR)
	}
	interrupts();
	return cancelState;
}

// phaseA change handler
//
void encoder::phaseAHandler(void) 
{
	digitalWrite(13, !digitalRead(13));

	if(digitalRead(3) == digitalRead(4))
		decCount();
	else
		incCount();
} 

void encoder::enterHandler(void) 
{
	volatile long time = millis();

	if (isEnterPinPressed())
		_enterStartTime = time;
	else if (_enterState == 3)
		_enterState = 0;
	else if (isTimeElapsed(time, ENTER_LONG_PUSH))
		_enterState = 2;
	else if (isTimeElapsed(time, ENTER_SHORT_PUSH))
		_enterState = 1;
}


// The one and only Global Encoder Object
encoder Encoder;


// Enter Function Interrupt Service Routine wrapper
void enterISR(void)
{
	Encoder.enterHandler();
}

// ALPS Encoder Function Interrupt Service Routine wrapper
void phaseAISR(void)
{
	Encoder.phaseAHandler();
}


