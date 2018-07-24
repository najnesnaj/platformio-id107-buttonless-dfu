# platformio-id107
nrf51822 ID107 smartwatch arduino buttonless dfu 

this is work in progress, that piggy backs on previous work by many people, mainly : @goran-mahovlic, @rogerclarkmelbourne, @curtpw, @Gordon, @micooke and in this case @arglurgl

I used code from https://github.com/dingari/ota-dfu-python.git in order to upload firmware with a bluetooth 4.0 dongle on a raspberry pi
code is included in the repository



included is  a  HOWTO on setting up platformio for the smartwatch
included is  a  HOWTO on using scripts to upload firmware 
included is  a  HOWTO on using bluez - linux - to set device to DFU mode 


some backgroundinfo can be found here : https://github.com/najnesnaj/smartband
and here: https://github.com/najnesnaj/ota-dfu-smartband



these is an example sketch under "src" which uses the "led switch" example to advertise a service

BLEService("19b10010e8f2537e4f6cd104768a1214");

if you write something to that service, it does not switch a led, but puts the watch in DFU mode   (buttonless DFU(device firmware update) mode)


-------------------------------------------------------------
you will find these modified SDK 12.3 files under directory nordic
-------------------------------------------------------------

in order to get to buttonless DFU I modified the bootloader

components/libraries/bootloader/dfu
nrf_dfu.c

 
 __WEAK bool nrf_dfu_enter_check(void)
 {
if (NRF_POWER->GPREGRET==0xBB){
       NRF_POWER->GPREGRET = 0; //set a different value to avoid loop
               return true;
	       }


--the bootloadercode  was for pca100028-board, I modified the Bootloader button to number 4, which corresponds to the side button of the watch -- this means that putting the watch into DFU mode is still possible with the side button -- you will have to include some code in your sketch to softreset the watch

 (the header file is under Nordic as well)







once in dfu-mode, you can use the uploadblue script to upload firmware via bluetooth

-----------------------------------------------------------------------
under the "src" directory you'll find the "arduino" program with some "nordic" flavours
-----------------------------------------------------------------------
I found a lot of usefull information on https://gitter.im/nRF51822-Arduino-Mbed-smart-watch/Lobby
----------------------------------------------




