// 
// If wrap is set the encoder value will roll over to 0 if greater than max and 
// roll to max if less than 0. If wrap is cleared the encoder value will be limited
// to operate with in the min max constraints with no roll over.
//

#pragma once

#include "Arduino.h"  // for digitalRead, digitalWrite, etc


#define EXTERNAL_INT	0
#define PINCHANCE_INT	1

#define CUI_DEBOUNCE		50

#define ENTER_SHORT_PUSH	50
#define ENTER_LONG_PUSH	1000

enum eType
{
	ALPS = 0,
	CUI = 1
};

//ISR wrappers
void alpsISR();
void cuiISR();
void enterISR();

class encoder
{
private:
	uint8_t	_aPin;
	uint8_t	_bPin;
	uint8_t	_ePin;

	uint8_t	_intA;
	uint8_t	_intE;

	uint8_t	_type;      // encoder type (ALPS or CUI)
	bool	_activeLow;	// reverses sense of _ePin
	uint8_t	_intMode;	// true if using PinChange interrupts

	int		_lastCount;   // last value of count, used to determine delta

	int		_min;
	int 	_max;
	bool 	_wrap;      // true if count should wrap

	volatile int _count;

	// 0 - enter inactive
	// 1 - "ok" enter pressed longer than SHORT but shorter than LONG
	// 2 - "cancel" detected in ISR
	// 3 - "cancel" detected in cancel() method
	volatile byte _enterState;

	volatile unsigned long	_lastUpdate;         // mSec of last update
	volatile unsigned long	_enterStartTime;  // mSec when enter active

public:

	byte getEncoderState();   // return the Encoder State

	encoder(void);
	void begin(uint8_t type, uint8_t encE, uint8_t encA, uint8_t encB, uint8_t intE, uint8_t intA);
	void begin(uint8_t type, uint8_t encE, uint8_t encA, uint8_t encB);
	void end(void);

	void setMin(int min)      { _min = min; }
	void setMax(int max)      { _max = max; }
	void setWrap(bool wrap)   { _wrap = wrap; }
	void setCount(int count)  { _count = _lastCount = count; }
	void clearCount(void)     { _count = _min; }
	int  getCount(void)       { return _count; }
	void setActiveLow(bool state);

	int  change(void);
	int  getDelta(void);

	byte getEnterState(void)    { return _enterState; }
	void clearEnterState(void)  { _enterState = 0; }
	bool ok(void);
	bool cancel(void);

private:
	inline void incCount(void) { if (++_count > _max) _count = (_wrap) ? _min : _max; }
	inline void decCount(void) { if (--_count < _min) _count = (_wrap) ? _max : _min; }
	inline bool isEnterPinPressed(void) { return _activeLow ? !digitalRead(_ePin) : digitalRead(_ePin); } 
	inline bool isTimeElapsed(unsigned long curTime, unsigned long duration) { return (curTime - _enterStartTime) > duration; }  

public:
	//Interrupt Handlers
	void enterHandler(void);
	void alpsHandler(void);
	void cuiHandler(void);


};

extern encoder Encoder;

