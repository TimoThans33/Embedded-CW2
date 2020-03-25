
# Embedded-CW2
This project includes the code for the second coursework of the module Embedded Systems at Imperial College. The goal was to implement a motor controller which controlled a brushless motor.
This is an example of a real time task and therefore the aim was to make efficient code. This is why we did bitcoin mining
alongside controlling the motor.
For this project we have used the NUCLEO L432KC hardware component.

## Specification

Main functions of this code:
* [Spin for a defined number of rotations and stop without overshooting]
* [Spin at a defined angular velocity]
* [Execute a bitcoin mining kernel in the background]
* [Make the motor play a tune as it works]

### Prerequisites

We have used the mbed cli as IDE because this was more efficient for testing code. Here are the install instructions. It is advised to install and build the project in a virtual environment. Assuming you have pip and python installed already

##Linux
'''
sudo apt install mercurial
'''
'''
pip install mbed-cli
'''
Install the GCC_ARM compiler
'''
sudo apt-get install gcc-arm-none-eabi binutils-arm-none-eabi gdb-arm-none-eabi openocd
'''
Configure the compiler location
'''
mbed config -G ARM_PATH "C:\Program Files\ARM"
'''
Then to make this directory active you might get away with
'''
mbed deploy
mbed new .
'''
If that does not work try this
'''
mbed new your_own_env
cd your_own_env
mbed add http://os.mbed.com/users/Geremia/code/Crypto
cp Embedded-CW2/* your_own_env
'''
Plug in the L432KC board and configure your mbed workspace
'''
mbed target detect
mbed toolchain GCC_ARM
'''
Now you are ready for launch.... just run below and the data from the board is shown in the terminal
'''
mbed compile -f --sterm
'''
