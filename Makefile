.PHONY: native clean

gpu_cfg_generator.exe: gpu_cfg_generator
	cp gpu_cfg_gen gpu_cfg_gen.exe

gpu_cfg_generator: gpu_cfg_generator.c gpu_cfg_generator.h
	../cosmopolitan/bin/cosmocc -o gpu_cfg_gen gpu_cfg_generator.c

native: gpu_cfg_generator.c gpu_cfg_generator.h
	$(CC) -o gpu_cfg_gen gpu_cfg_generator.c -Wall
	
	
clean :
	rm gpu_cfg_gen gpu_cfg_gen.aarch64.elf gpu_cfg_gen.com.dbg
