#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>

#include <mach/hardware.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/clock.h>

#include <plat/regs-serial.h>
#include <plat/cpu_rework.h>


static struct cpu_table *cpu;

//Return a pointer to the cpu type.
static struct cpu_table * 
				__init	s3c_lookup_cpu(
							unsigned long idcode,
							struct cpu_table *tab,
							unsigned int count) {

	for(;count!=0;count--,tab++) {
		if((idcode & tab->idmask) == (tab->idcode & tab->idmask)) {
			return tab;
		}
	}

	return NULL;
}

/* 
 * Lookup the cpu table and call the initialization functions for
 * the particular cpu
 */
void __init s3c_init_cpu_rework(unsigned long idcode,
		struct cpu_table *cputab,
		unsigned int cputab_size) {

	cpu = s3c_lookup_cpu(idcode,
						cputab,
						cputab_size
						);

	if(cpu == NULL) {
		printk(KERN_ERR "Unknown CPU type 0x%08lx\n",idcode);
		panic("Unknown S3C24XX CPU");
	}

	printk("CPU %s (id 0x%08lx)\n",cpu->name, idcode);

	if(cpu->map_io == NULL || cpu->init == NULL) {
		printk(KERN_ERR "CPU %s support not enabled\n",cpu->name);
		panic("Unsupported Samsung CPU");
	}

	cpu->map_io();
}

/*
 * 	s3c24xx_init_clocks_rework
 *
 * 	Initialize the clock subsystem and associated information
 * 	form the given master value.
 *
 * 	xtal = 0 -> Use default pll crystal value (normally 12Mhz)
 * 	!=0 -> PLL crystal value in hz.
 *
 * 	TODO: Rewrite the clock tree setup.
 */
void __init s3c24xx_init_clocks_rework(int xtal) 
{
	if(xtal == 0)
		xtal = 12 * 1000 * 1000;

	
	if(cpu == NULL)
		panic("s3c24xx_init_clocks: No cpu setup?\n");

	if(cpu->init_clocks == NULL)
		panic("s3c24xx_init_clocks: cpu has no clock init\n");
	else
		(cpu->init_clocks)(xtal);
}
