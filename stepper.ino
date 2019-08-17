


#include <vector>
#include <queue>
#include <memory>
#include <array>

#include <signal.h>
#include <limits>
#include <cmath> // isnan


//#define DEBUG

#ifdef DEBUG
#define debugln(x) Serial.println(x)
#define debug(x) Serial.print(x)

#else

#define debugln(x)
#define debug(x)

#endif

using namespace std;

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;

//the pins for the stepper motors
int STEP = 2;
int DIR = 3;
int STEP2 = 4;
int DIR2 = 5;
int STEP3 = 6;
int DIR3 = 7;

int stepLength = 100 / 2 * 10;
int phaseSize = 200;
float phase = 0;
float speed = 1;

bool absolutePositioning = true;

unsigned long previousMicros;
int previousCommand = 'G';
int previousCommandNumber = 1;

struct Location {
	long x, y, z, e;

	long &operator[] (int index) {
		return (&x)[index];
	}

	size_t size() {
		return 4;
	}
};

Location position;
const array<int, 4> axisPins = {STEP, STEP2, STEP3, -1}; // -1 means unset
const array<int, 4> directionPins {DIR, DIR2, DIR3, -1}; // -1 means unset
const array<int, 4> stepsPerMM = {300, 300, 300, 300};
const array<int, 4> maxSpeed = {300 / 10, 300 / 10, 300 / 10, 300};

struct Argument {
	char name;
	float value = 0;
	bool hasValue = true;
};

void printChar(char c) {
	char output[2] = {(char) c, (char) 0};
	Serial.print(output);
}


// A function that reads float (both positive and negative) from the serial port
float readFloat() {
	auto p = Serial.peek();
	if ((p >= '0' && p <= '9') || p == '.') {
		return Serial.parseFloat();
	}
	else if (p == '-') {
		Serial.read(); //Throw away the - char
		return -Serial.parseFloat();
	}
	else {
		return std::numeric_limits<float>::quiet_NaN();
	}
}


struct Command {
	char command;
	virtual ~Command() {}

	int number;
	std::vector<Argument> arguments;
	bool initialized = false;

	void parseArguments() {
		char c;
		while (c != 10) {
			while (Serial.available() <= 0) {
				//Wait for input
			}
			c = Serial.read();
			if (c >= 'A' && c <= 'Z') {
				Argument argument;
				argument.name = c;
				auto value = readFloat();
				if (std::isnan(value)) {
					argument.hasValue = false;
				}
				else {
					argument.value = value;
				}
				arguments.push_back(argument);
			}
		}
	}

	Argument *getArgument(char name) {
		for (auto &arg : arguments) {
			if (arg.name == name) {
				return &arg;
			}
		}
		return nullptr;
	}

	float getArgumentValue(char name, float def) {
		auto arg = getArgument(name);
		if (arg) {
			return arg->value;
		}
		else {
			return def;
		}
	}

	float getArgumentValue(char name, float def, float multiplier) {
		auto arg = getArgument(name);
		if (arg) {
			return arg->value * multiplier;
		}
		else {
			return def;
		}
	}


	void print() {
		printChar(command);
		Serial.print(number);

		Serial.print(" ");

		for (auto arg : arguments) {
			printChar(arg.name);
			Serial.print(arg.value);
			Serial.print(" ");

		}

		switch (command) {
		case 'G':
			switch (number) {
			case 1:
				Serial.print("linear motion");
				if (absolutePositioning) {
					Serial.print(" absolute positioning");
				}
				else {
					Serial.print(" relative positioning");
				}
				break;
			}
			break;
		}

		Serial.println("");
	}

	virtual bool operator()(int dt) = 0;
	virtual void init() = 0;
};

struct SingleCommand: Command {
	bool operator()(int dt) override {
		return true;
	}
};

#define fori(i, o) for(size_t i = 0; i < o.size(); ++i)

struct G1Command: public Command {
	Location target;
	Location from;
	Location direction;
	long operationLength = 1000000;
	long realLength = 0;
	long progress = 0;

	long speedAmount = 100;
	long rampLength = 100000;

	bool operator()(int dt) override {
		realLength += dt;
		if (realLength < rampLength) {
			speedAmount = 100 * realLength / rampLength;
		}
		else {
			speedAmount = 100;
		}
		
		// Todo: Make sure that step not is to long here
		
		progress += dt * speedAmount / 100;
		debug("dt ");
		debugln(dt);

		debug("progress:\t");
		debugln(100 * progress / operationLength);

		debug("\tspeed %:\t");
		debug(speedAmount);
		fori(i, direction) {
			position[i] = from[i] + direction[i] * progress / operationLength;
			if (axisPins[i] > -1) {
				int pinValue = abs(position[i] % 2);
				digitalWrite(axisPins[i], pinValue);
				digitalWrite(led, pinValue);
				debug("axis:\t");
				debug(i);
				debug("\tposition\t");
				debug(position[i]);

				debug("\tpin number\t");
				debug(axisPins[i]);
				debug("\tpinvalue\t");
				debugln(abs(position[i] % 2));
			}
		}

		return progress > operationLength;
	}

	void init() override {
		Serial.println("started linear motion");

		if (absolutePositioning) { //Absolute positioning
			target.x = getArgumentValue('X', position.x, stepsPerMM[0]);
			target.y = getArgumentValue('Y', position.y, stepsPerMM[1]);
			target.z = getArgumentValue('Z', position.z, stepsPerMM[2]);
			target.e = getArgumentValue('E', position.e, stepsPerMM[3]);

			if (target.x == position.x && target.y == position.y && target.z == position.z && target.e == position.e) {
				Serial.println(F("Already at target position"));
			}
		}
		else {
			target.x = position.x + getArgumentValue('X', 0, stepsPerMM[0]);
			target.y = position.y + getArgumentValue('Y', 0, stepsPerMM[1]);
			target.z = position.z + getArgumentValue('Z', 0, stepsPerMM[2]);
			target.e = position.e + getArgumentValue('E', 0, stepsPerMM[3]);
		}

		debugln("moving to: ");
		debugln(target.x);
		debugln(target.y);
		debugln(target.z);
		debugln(target.e);

		fori(i, direction) {
			from[i] = position[i];
			debugln("loop");
			debugln(position[i]);
			debugln(from[i]);
			direction[i] = target[i] - position[i];
			if (directionPins[i] > -1) {
				digitalWrite(directionPins[i], direction[i] > 0);
			}
		}

		debugln("moving from: ");
		debugln(from.x);
		debugln(from.y);
		debugln(from.z);
		debugln(from.e);

		debugln("direction : ");
		debugln(direction.x);
		debugln(direction.y);
		debugln(direction.z);
		debugln(direction.e);

		operationLength = 0;
		fori (i, direction) {
			auto l = abs(direction[i]) * (1000 * 3 / 2);// * 1000 / maxSpeed[i];
			if (l > operationLength) {
				operationLength = l;
			}
		}
//		Serial.print("operation length");
//		Serial.println(operationLength);
	}
};

struct PositioningCommand: public SingleCommand {
	PositioningCommand(bool positioningType): positioningType(positioningType) {}
	void init() override {
		arguments.clear();
		Serial.print(F("Setting absolute positioning to "));
		Serial.println(positioningType);
		absolutePositioning = positioningType;
	}

	bool positioningType;
};


std::queue<unique_ptr<Command>> commands;


// Reads input form serial in and create a queue with commands
void processInput() {
	if (Serial.available() <= 0) {
		return;
	}
	while (Serial.available() > 0) {
		// read the incoming byte:
		int c = Serial.read();
		c = toupper(c);

		if (c == ';') {
			while (c != 10 && Serial.available() > 0) {
				c = Serial.read();
			}
			debugln("removed comment");
		}
		else if (c == '(') {
			while (c != ')' && Serial.available() > 0) {
				c = Serial.read();
			}
			debugln("removed comment");
		}
		else if ((c >= 'A' && c <= 'Z') || c == ' ') {
			Command *command = nullptr;
			int number;

			Argument argument; //If this is is shorthand notation, values is stored here
			argument.name = 0;

			if (c >= 'X' && c <= 'Z') {
				argument.name = c;
				argument.value = readFloat();
				if (std::isnan(argument.value)) {
					argument.hasValue = false;
				}

				c = previousCommand;
				number = previousCommandNumber;
				Serial.println("reusing previous command ");

				char prevString[] = {(char)c, 0};
				Serial.print(prevString);
				Serial.println(previousCommandNumber);

			}
			else {
				previousCommand = c;
				previousCommandNumber = number = Serial.parseInt();
			}


			switch (c) {
			case 'G':
				switch (number) {
				case 1:
					command = new G1Command;
					break;
				case 90:
					command = new PositioningCommand(true);
					break;
				case 91:
					command = new PositioningCommand(false);
					break;
				}
				break;
			}

			if (command) {
				command->command = c;
				command->number = number;

				if (argument.name != 0) {
					command->arguments.push_back(argument);
				}
				command->parseArguments();

				command->print();
				commands.push(unique_ptr<Command>(command));
			}
		}
	}
}


// the setup routine runs once when you press reset:
void setup() {
	// initialize the digital pin as an output.
	pinMode(led, OUTPUT);

	for (auto pin: axisPins) {
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}
	for (auto pin: directionPins) {
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
	}
	digitalWrite(DIR, LOW);

	previousMicros = micros();
}



// the loop routine runs over and over again forever:
void loop() {
	processInput();

	if (!commands.empty()) {
		auto &command = *commands.front();
		if (!command.initialized) {
			command.init();
			command.initialized = true; //Only do once
			previousMicros = micros();
		}

		auto currentMicros = micros();
		auto d = currentMicros - previousMicros;
		auto finished = command(d); // Do command stuff

		if (finished) {
			commands.pop();
			Serial.println("command finished");
		}
		previousMicros = currentMicros;
	}
}



// This is to avoid error messages when compiling from command line
namespace std {
void __throw_bad_alloc()
{
	Serial.println(F("Unable to allocate memory"));
	while (true) ; // for noreturn, a better alternative than 
 //abort(); // for "noreturn"
}
void __throw_length_error( char const*e )
{
	Serial.print(F("Length Error :"));
	Serial.println(e);
	while (true) ;
  //abort();
}
}

