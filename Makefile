.PHONY: native clean

COSMOCC=../cosmopolitan

gpu_cfg_generator.exe: gpu_cfg_generator
	cp gpu_cfg_gen gpu_cfg_gen.exe

GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")$(shell git diff --quiet && git diff --cached --quiet || echo "-dirty")

gpu_cfg_generator: gpu_cfg_generator.c gpu_cfg_generator.h
	$(COSMOCC)/bin/cosmocc -o gpu_cfg_gen *.c -I ./ -DGIT_HASH=\"$(GIT_HASH)\"

native: gpu_cfg_generator.c gpu_cfg_generator.h
	$(CC) -o gpu_cfg_gen *.c -Wall -I ./ -g -DGIT_HASH=\"$(GIT_HASH)\"
	
	
clean :
	rm gpu_cfg_gen gpu_cfg_gen.aarch64.elf gpu_cfg_gen.com.dbg
