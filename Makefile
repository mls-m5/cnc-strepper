
all: upload

verify:
	~/Prog/Program/arduino-1.8.9/arduino  --verify stepper.ino | sed "s/stepper:/stepper.ino:/g"
	
upload:
	~/Prog/Program/arduino-1.8.9/arduino  --upload stepper.ino | sed "s/stepper:/stepper.ino:/g"

test: testing.cpp stepper.ino
	c++ testing.cpp -std=c++14 -o test -g -pthread