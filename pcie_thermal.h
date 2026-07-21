#ifndef __PCIE_THERMAL_H
#define __PCIE_THERMAL_H

#include <gpu_cfg_generator.h>


struct default_pcie_thermal_cfg {
	struct gpu_cfg_descriptor descriptor;

	struct gpu_block_header hdr0;
	enum gpu_pcie_cfg pcie_cfg;

	struct gpu_block_header hdr1;
	struct gpu_cfg_fan fan0_cfg;

	struct gpu_block_header hdr2;
	struct gpu_cfg_fan fan1_cfg;

	struct gpu_block_header hdr3;
	enum gpu_vendor vendor;

	struct gpu_block_header hdr4;
	struct gpu_cfg_gpio     gpio0;
	struct gpu_cfg_gpio     gpio1;
	struct gpu_cfg_gpio     gpio2;
	struct gpu_cfg_gpio     gpio3;
	struct gpu_cfg_gpio     gpio_edpaux;
	struct gpu_cfg_gpio     gpio_vsys;
	struct gpu_cfg_gpio     gpio_fan;
	struct gpu_cfg_gpio     gpu_3v_5v_en;

	struct gpu_block_header hdr5;
	struct gpu_cfg_thermal therm;

	struct gpu_block_header hdr6;
	struct gpu_cfg_custom_temp custom_temp1;
	struct gpu_cfg_custom_temp custom_temp2;
	struct gpu_cfg_custom_temp custom_temp3;


	struct gpu_block_header hdr7;
	struct gpu_subsys_serial pcba_serial;

} __packed;


extern struct default_pcie_thermal_cfg pcie_thermal_cfg;

void pcie_thermal_write_pcb(char * serial);

#endif /* __PCIE_THERMAL_H */