
all: upload

verify:
	arduino  --verify stepper.ino | sed "s/stepper:/stepper.ino:/g"
	
upload:
	arduino  --upload stepper.ino | sed "s/stepper:/stepper.ino:/g"

test: testing.cpp stepper.ino
	c++ testing.cpp -std=c++14 -o test -g -pthread
