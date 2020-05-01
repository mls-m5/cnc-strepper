
all: upload

verify:
	platformio run
	#arduino  --verify stepper.ino | sed "s/stepper:/stepper.ino:/g"
	
upload:
	platformio run --target upload
	arduino  --upload stepper.ino | sed "s/stepper:/stepper.ino:/g"

sim: #testing.cpp src/mill.cpp
	c++ testing.cpp -std=c++17 -o sim -g -pthread
