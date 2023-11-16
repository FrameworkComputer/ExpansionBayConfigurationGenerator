# build notes

uses
https://github.com/jart/cosmopolitan

extract cosmopolitan one folder up

run make


to use on windows rename

gpu_cfg_gen to gpu_cfg_gen.exe



# Running 
when the application is run, pass in the serial, and for the GPU, the PCB serial. 
The application will generate a .bin file in the same directory with the EEPROM contents
## Generate GPU Serial

./gpu_cfg_gen -g -s FRAKMBCP81331ASSY0 -p FRAGMASP81331PCB00


## Generate SSD

./gpu_cfg_gen -d -s FRAKMBCP81331ASSY0
