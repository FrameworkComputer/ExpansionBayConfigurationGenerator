# build notes

Uses [cosmpopolitan](https://github.com/jart/cosmopolitan).

Extract cosmopolitan release zip one folder up.

```sh
run make
```

To use it on windows, rename `gpu_cfg_gen` to `gpu_cfg_gen.exe`.



# Running
When the application is run, pass in the serial, and for the GPU, the PCB serial.
The application will generate a `.bin` file in the same directory with the EEPROM contents.

## Generate GPU Serial

```
./gpu_cfg_gen -g -s FRAKMBCP81331ASSY0 -p FRAGMASP81331PCB00
```


## Generate SSD

```
./gpu_cfg_gen -d -s FRAKMBCP81331ASSY0
```

## Different file name

By default the generated file is called `eeprom.bin`, here's how to use a different one:

```
./gpu_cfg_gen -d -s FRAKMBCP81331ASSY0 -o ssd.bin
```

## Read EEPROM binary

To double-check you can read the binary back from EEPROM and analyze it with the tool:

```sh
./gpu_cfg_gen -i eeprom.bin

# More verbose output
./gpu_cfg_gen -i eeprom.bin -v
```

# Build natively

While the regular build builds a single executable that runs on Linux and
Windows, for development purpose you might want to build using native tooling.

```sh
# With GCC
make native CC=gcc

# With clang
make native CC=clang
```
