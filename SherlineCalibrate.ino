// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       SherlineCalibrate.ino
    Created:	12/11/2018 7:54:12 PM
    Author:     tls-mobile7\tomh
*/

#include "encoder.h"

// Encoder Pins
#define pinEncE		2
#define pinEncA		3
#define pinEncB		4

// Motor Controller Pins
#define pinStep		 5
#define pinDirection 6
#define pinMS1		 7
#define pinMS2		 8
#define pinEnable	 12
#define pinLED		 13


// Stepper Motor Wiring
//   Phase	Tronxy	Portescap	Applied Motion
//            
//   -----	-------	---------	--------------
//	   A	Blk/Grn	 Red/Gry	Org/Blk short Or/Wh to Bk/Wh
//	   B	Blu/Red	 Yel/Blk	Red/Yel short Rd/Wh to Yl/Wh

//Declare variables for functions
char _UserInput;
int x;
int y;
int state;
int _encoderCount;

void setup() 
{
	pinMode(pinLED, OUTPUT);
	
	pinMode(pinStep, OUTPUT);
	pinMode(pinDirection, OUTPUT);
	pinMode(pinMS1, OUTPUT);
	pinMode(pinMS2, OUTPUT);
	pinMode(pinEnable, OUTPUT);

	ResetMotorPins(); 

	//pinMode(pinEncE, INPUT);
	//pinMode(pinEncA, INPUT);
	//pinMode(pinEncB, INPUT);
	Encoder.setMax(1000000);
	Encoder.setMin(-1000000);
	Encoder.begin(pinEncE, pinEncA, pinEncB);

	Serial.begin(115200);
	Serial.println("Begin motor control");
	Serial.println();
	
	WriteEncoderCount();
}

void loop()
{
	//	digitalWrite(pinLED, );
	//if (UpdateEncoder())
	//	WriteEncoderCount();

	if (Serial.available()) 
	{
		_UserInput = Serial.read();		//Read user input and trigger appropriate function
		digitalWrite(pinEnable, LOW);	//Pull enable pin low to allow motor control

		switch(_UserInput)
		{
		case 'r':
			_encoderCount = 0;
			WriteEncoderCount();
			break;
		case '1':
			StepForwardDefault();
			PrintEncoderCount();
			break;
		case '2':
			ReverseStepDefault();
			PrintEncoderCount();
			break;
		case '3':
			SmallStepMode(1);
			PrintEncoderCount();
			break;
		case '4':
			SmallStepMode(0);
			PrintEncoderCount();
			break;
		case '5':
			ForwardBackwardStep();
			PrintEncoderCount();
			break;
		case 'e':
			digitalWrite(pinEnable, LOW);
			PrintEncoderCount();
			break;
		case '?':
			DisplayHelp();
			PrintEncoderCount();
			break;
		case 'x':
			PrintEncoderCount();
			break;
		case 'z':
			Encoder.setCount(0);
			PrintEncoderCount();
			break;
		default:
			Serial.println("Invalid option entered.");
			break;
		}

//		Serial.println("Enter new option:");
//		Serial.println();

		ResetMotorPins();
	}

	//if (Encoder.getDelta() > 0)
	//{
	//	Serial.print("Encoder Count: ");
	//	Serial.println(Encoder.getCount());
	//}

}

void PrintEncoderCount()
{
	Serial.print("Encoder Count = ");
	Serial.println(Encoder.getCount());
}

bool UpdateEncoder()
{
	static bool lastEncA;
	bool curEncA = digitalRead(pinEncA);
	bool curEncB = digitalRead(pinEncB);

	if (curEncA == lastEncA)
		return false;

	if (curEncA)
	{
		if (curEncB)
			_encoderCount++;
		else
			_encoderCount--;
	}
	else
	{
		if (curEncB)
			_encoderCount--;
		else
			_encoderCount++;
	}

	lastEncA = curEncA;

	return true;
}

void WriteEncoderCount()
{
	Serial.print("EncoderCount = ");
	Serial.println(_encoderCount);
}

// Default microstep mode function
void StepForwardDefault()
{
	Serial.println("Moving forward at default step mode.");
	digitalWrite(pinDirection, LOW);	//Pull direction pin low to move "forward"
	int delayVal = 1;
	for (x = 0; x < 200; x++)			//Loop the forward stepping enough times for motion to be visible
	{
		if (x==0 | x == 190)
			delayVal = 10;
		else if (delayVal > 1)
			delayVal -= 1;

		digitalWrite(pinStep, HIGH);	//Trigger one step forward
		delay(delayVal);
		digitalWrite(pinStep, LOW);		//Pull step pin low so it can be triggered again
		delay(delayVal);
	}
}

// Reverse default microstep mode function
void ReverseStepDefault()
{
	Serial.println("Moving in reverse at default step mode.");
	digitalWrite(pinDirection, HIGH);	//Pull direction pin high to move in "reverse"
	for (x = 0; x < 200; x++)			//Loop the stepping enough times for motion to be visible
	{
		digitalWrite(pinStep, HIGH);	//Trigger one step
		delay(1);
		digitalWrite(pinStep, LOW);		//Pull step pin low so it can be triggered again
		delay(1);
	}
}

// 1/8th microstep foward mode function
void SmallStepMode(int dir)
{
	Serial.println("Stepping at 1/8th microstep mode.");
	digitalWrite(pinDirection, dir);	//Pull direction pin low to move "forward"
	digitalWrite(pinMS1, HIGH);			//Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
	digitalWrite(pinMS2, HIGH);
	for (x = 0; x < 1600; x++)			//Loop the forward stepping enough times for motion to be visible
	{
		digitalWrite(pinStep, HIGH);	//Trigger one step forward
		delay(5);
		digitalWrite(pinStep, LOW);		//Pull step pin low so it can be triggered again
		delay(5);
	}
}

// Forward/reverse stepping function
void ForwardBackwardStep()
{
	Serial.println("Alternate between stepping forward and reverse.");
	for (x = 1; x < 5; x++)  //Loop the forward stepping enough times for motion to be visible
	{
		//Read direction pin state and change it
		state = digitalRead(pinDirection);
		if (state == HIGH)
		{
			digitalWrite(pinDirection, LOW);
		}
		else if (state == LOW)
		{
			digitalWrite(pinDirection, HIGH);
		}

		for (y = 1; y < 1000; y++)
		{
			digitalWrite(pinStep, HIGH); //Trigger one step
			delay(1);
			digitalWrite(pinStep, LOW); //Pull step pin low so it can be triggered again
			delay(1);
		}
	}
}

// Reset Easy Driver pins to default states
void ResetMotorPins()
{
	digitalWrite(pinStep, LOW);
	digitalWrite(pinDirection, LOW);
	digitalWrite(pinMS1, LOW);
	digitalWrite(pinMS2, LOW);
//	digitalWrite(EN, HIGH);
}

void DisplayHelp()
{
	//Print function list for user selection
	Serial.println("Enter number for control option:");
	Serial.println("  1 Turn at default microstep mode.");
	Serial.println("  2 Reverse direction at default microstep mode.");
	Serial.println("  3 Turn at 1/8th microstep mode.");
	Serial.println("  4 Step forward and reverse directions.");
	Serial.println("  ? Display Help");
	Serial.println();
}


