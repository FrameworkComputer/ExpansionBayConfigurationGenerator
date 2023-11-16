gpu_cfg_generator.o : gpu_cfg_generator.c gpu_cfg_generator.h
	../cosmopolitan/bin/cosmocc -o gpu_cfg_gen gpu_cfg_generator.c
	
	
clean :
	rm gpu_cfg_gen gpu_cfg_gen.aarch64.elf gpu_cfg_gen.com.dbg
