# Project-Ouroboros

ATmega8 USBasp programmers available for cheap on eBay and aliexpress can be reprogrammed with a bootloader or standalone code.
With three of the microcontrollers pins as well as power and ground connected to the programming header they are an inexpensive
way to realize an embedded project.

This repo contains USBaspLoader bootloaders modified to run on one of these programmers, and a few demo programs.
I originally worked on this project in 2011, and updated my work in 2019. Visit the following link for an in depth write-up.

https://jethomson.wordpress.com/2011/08/18/project-ouroboros-reflashing-a-betemcu-usbasp-programmer/

N.B. These files use a fuse setting that sets the brown-out detection level appropriate for a 3.3V board, if your board is 5V this fuse setting won't hurt your board, but it won't properly detect a brown-out condition. If you are using a 5V board you can change the low fuse from 0xBF to 0x3F (e.g. lfuse:w:0xBF:m to lfuse:w:0x3F:m).

**Summary of Methods to Enter Programming Mode**
External programmer: connect the holes of J2 together for the duration of the program upload.
Vanilla USBaspLoader: connect PD7 to ground, reset, upload program, disconnect PD7 from ground.
Alternate USBaspLoader: 1) reset, upload program before bootloader times out. 2) press and hold PD7 pushbutton, reset, release PD7 pushbutton, upload program
