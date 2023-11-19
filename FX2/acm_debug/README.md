
# doctored SAG1021I FX2 firmware



## Prerequisites 

libfx2 https://github.com/whitequark/libfx2

## Build

- ensure the LIBFX2 path is correct in the Makefile
- `make` does what you'd expect, output is *doctored-sag1021I.ihex*
- `make load` will build the project and load it to the FX2 

