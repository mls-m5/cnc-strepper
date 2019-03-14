/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
 */


#include <vector>
#include <queue>
#include <memory>
#include <array>

#include <signal.h>


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
int STEP = 2;
int DIR = 3;
int STEP2 = 4;
int DIR2 = 5;
int stepLength = 100 / 2 * 10;
int phaseSize = 200;
float phase = 0;
float speed = 1;

bool absolutePositioning = true;

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
const array<int, 4> axisPins = {STEP, STEP2, -1, -1};
const array<int, 4> directionPins {DIR, DIR2, -1, -1};

struct Argument {
	char name;
	float value = 0;
	bool hasValue = true;
};

void printChar(char c) {
	char output[2] = {(char) c, (char) 0};
	Serial.print(output);
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
			//if (c == 10) {
			//  break;
			//}
			if (c >= 'A' && c <= 'Z') {
				Argument argument;
				argument.name = c;
				//Serial.println("new argument");
				auto p = Serial.peek();
				if ((p >= '0' && p <= '9') || p == '.') {
					argument.value = Serial.parseFloat();
				}
				else {
					argument.hasValue = false;
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
	long progress = 0;

	bool operator()(int dt) override {
		progress += dt;
		debug("dt ");
		debugln(dt);

		debug("progress:\t");
		debugln(100 * progress / operationLength);
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
			target.x = getArgumentValue('X', position.x);
			target.y = getArgumentValue('Y', position.y);
			target.z = getArgumentValue('Z', position.z);
			target.e = getArgumentValue('E', position.e);

			if (target.x == position.x && target.y == position.y && target.z == position.z && target.e == position.e) {
				Serial.println(F("Already at target position"));
			}
		}
		else {
			target.x = position.x + getArgumentValue('X', 0);
			target.y = position.y + getArgumentValue('Y', 0);
			target.z = position.z + getArgumentValue('Z', 0);
			target.e = position.e + getArgumentValue('E', 0);
		}

		Serial.println("moving to: ");
		Serial.println(target.x);
		Serial.println(target.y);
		Serial.println(target.z);
		Serial.println(target.e);


		fori(i, direction) {
			from[i] = position[i];
			Serial.println("loop");
			Serial.println(position[i]);
			Serial.println(from[i]);
			direction[i] = target[i] - position[i];
			if (directionPins[i] > -1) {
				digitalWrite(directionPins[i], direction[i] > 0);
			}
		}

		Serial.println("moving from: ");
		Serial.println(from.x);
		Serial.println(from.y);
		Serial.println(from.z);
		Serial.println(from.e);

		Serial.println("direction : ");
		Serial.println(direction.x);
		Serial.println(direction.y);
		Serial.println(direction.z);
		Serial.println(direction.e);

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

void processInput() {
	if (Serial.available() <= 0) {
		return;
	}
	while (Serial.available() > 0) {
		// read the incoming byte:
		int c = Serial.read();

		if (c == '#') {
			while (c != 10 && Serial.available() > 0) {
				c = Serial.read();
			}
			Serial.println("removed comment");
		}
		if (c == '(') {
			while (c != ')' && Serial.available() > 0) {
				c = Serial.read();
			}
			Serial.println("removed comment");
		}
		else if (c >= 'A' && c <= 'Z') {
			Command *command = nullptr;
			int number = Serial.parseInt();

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
				command->parseArguments();

				command->print();
				commands.push(unique_ptr<Command>(command));
			}
		}
	}
}

unsigned long previousMicros;

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
		auto finished = command(d);

		if (finished) {
			commands.pop();
			Serial.print("command finished");
		}
		previousMicros = currentMicros;
	}
}


namespace std {
void __throw_bad_alloc()
{
	Serial.println("Unable to allocate memory");
}

void __throw_length_error( char const*e )
{
	Serial.print("Length Error :");
	Serial.println(e);
}
}
