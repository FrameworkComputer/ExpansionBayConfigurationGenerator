#ifndef __PCIE_H
#define __PCIE_H

#include <gpu_cfg_generator.h>


struct default_pcie_cfg {
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

} __packed;


extern struct default_pcie_cfg pcie_accessory_cfg;


#endif /* __PCIE_H */