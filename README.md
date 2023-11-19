# doctored SAG1021I

I received 2 SAG1021I AWGs with the purchase of my oscilloscope.
But my scope doesn't actually support them.

Although they appear as USB Test and Measruement (IVI) Devices when connected to a USB host the SCPI interface is not documented and non-standard.

This is an attempt to make them useful with some custom gateware for the FPGA and firmware for the FX2 USB interface.

## ⚠️⚠️⚠️ NB: This is presently a work in progress and probably not useful except for educational purposes ⚠️⚠️⚠️

Oh, I also know nothing about FPGAs so I'm using this is a learning exercise, the code quality is likely very low. 

## Prerequisites 

### For FPGA gateware
Intel Quartus Lite


### For FX2 firmware
libfx2 https://github.com/whitequark/libfx2
