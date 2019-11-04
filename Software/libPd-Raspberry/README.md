### E256 libpd patches
Patches made to work with the E256 eTextile matrix sensor connected via USB to a Raspberry-pi

## Hardware
 - [Raspbeary-PI 3](https://www.raspberrypi.org)
 - [I2S sound card] ...

## To make your own patches you can follow this instructions
  - Cloning Puredata on your Raspberry
    - git clone ...
	- sudo apt install build-essential autoconf automake libtool gettext git libasound2-dev libjack-jackd2-dev libfftw3-3 libfftw3-dev tcl tk
	- cd Pd
	- ./autogen.sh
	- ./configure --enable-fftw
	- make
	- sudo make install

  - Add the following library with the board manager (menu->...)
    - comport
    - mrpeach

## Auto start your project at boot
  - vim /etc/rc.local
  - Add (just before exit 0) pd -nogui -alsa -nomidi -path /home/pi/Documents/Pd/externals/ /path_to_your_pdFile.pd &
