
all: upload

verify:
	~/Prog/Program/arduino-1.8.8/arduino  --verify stepper.ino | sed "s/stepper:/stepper.ino:/g"
	
upload:
	~/Prog/Program/arduino-1.8.8/arduino  --upload stepper.ino | sed "s/stepper:/stepper.ino:/g"