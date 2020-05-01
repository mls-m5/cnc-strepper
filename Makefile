
all: upload

verify:
	platformio run
	#arduino  --verify stepper.ino | sed "s/stepper:/stepper.ino:/g"
	
upload:
	platformio run --target upload
	arduino  --upload stepper.ino | sed "s/stepper:/stepper.ino:/g"

simulation:
   make -C simulation
