
# Embedded-CW2
This project includes the code for the second coursework of the module Embedded Systems at Imperial College. The goal was to implement a motor controller which controlled a brushless motor.
This is an example of a real time task and therefore the aim was to make efficient code. This is why we did bitcoin mining alongside controlling the motor.
For this project we have used the NUCLEO L432KC hardware component.

## Specification

Main functions of this code:
* Spin for a defined number of rotations and stop without overshooting
* Spin at a defined angular velocity
* Execute a bitcoin mining kernel in the background capped at 5000 hashrate
* Make the motor play a tune as it works

## Commands
This code takes in some serial commands the format of these serial commands is as follows. All the commands have to finish with an enter.
* R-{?} (This will rotate backwards for ? rotations)
* V{?} (Changes the velocity up to a maximum of 20)
* T{?} (Takes in a melody of four notes in 5 different octaves and plays every note for 0.4 seconds)
* K{16} (Defines a new key for the hashing algorithm)

## Prerequisites

We have used the mbed cli as IDE because this was more efficient for testing code. Here are the install instructions. Assuming you have pip and python installed already. It is advised to install and build the project in a virtual environment.

## Linux
```
sudo apt install mercurial
```
```
pip install mbed-cli
```
Install the GCC_ARM compiler
```
sudo apt-get install gcc-arm-none-eabi binutils-arm-none-eabi gdb-arm-none-eabi openocd
```
Then to make this directory active you might get away with
```
mbed deploy
mbed new .
```
If that does not work try this
```
mbed new your_own_env
cd your_own_env
mbed add http://os.mbed.com/users/Geremia/code/Crypto
cp Embedded-CW2/* your_own_env
```
Plug in the L432KC board and configure your mbed workspace
```
mbed target detect
mbed toolchain GCC_ARM
```
Now you are ready for launch.... just run below and the data from the board is shown in the terminal
```
mbed compile -f --sterm
```
Alternatively you can use screen from Linux after compiling
```
screen your_device 9600
```
