/*
 * An exercise on porting.
 */


#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/serial_core.h>
//#include <linux/platform_device.h>
//#include <linux/io.h>

#include <mach/hardware.h>
#include <mach/regs-gpio.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <asm/system_info.h>
#include <asm/system_misc.h>

#include "common.h"
#include <plat/cpu.h>
#include <plat/cpu_rework.h>

//#include <plat/cpu-freq.h>
#include <plat/s3c2410.h>
#include <plat/s3c244x.h>
#include <plat/regs-serial.h>
#include <mach/regs-clock.h>
#include <asm/io.h>
#include <plat/pll.h>

/* Default control,line and fifo control reg values */

/* 
 * Defaults: 
 * Rx & Tx in interrupt mode.
 * Rx & Tx in level mode.
 * Rx FIFO in Timeout mode.
 *
 */
#define UCON (S3C2410_UCON_DEFAULT) 
#define ULCON (S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB)

/* Set fifo mode to Rx and trigger at 8 bytes. Enable Fifomode. */
#define UFCON (S3C2440_UFCON_RXTRIG8|S3C2410_UFCON_FIFOMODE)

static struct map_desc mini2440_rework_iodesc[] __initdata = {
	// Nothing to declare. Use init map which contains code
	// to init the cpu.
};

/*
 * 	S3C2440 consists of 3 independent UART's each of which can
 * 	operate either in interrupt or in DMA mode. Create a 3 sized
 * 	array to hold the states of these ports.
 */
static struct s3c2410_uartcfg mini2440_rework_uartcfgs[] __initdata = {
	[0] = {
		.hwport = 0,
		.flags = 0,
		.ucon = UCON, //Set default UART control reg values.
		.ulcon = ULCON, //Set default UART line control reg values.
		.ufcon = UFCON, //Set default UART fifo control reg values.
	},
	[1] = {
		.hwport = 1,
		.flags = 0,
		.ucon = UCON, //Set default UART control reg values.
		.ulcon = ULCON, //Set default UART line control reg values.
		.ufcon = UFCON, //Set default UART fifo control reg values.
	},
	[2] = {
		.hwport = 2,
		.flags = 0,
		.ucon = UCON, //Set default UART control reg values.
		.ulcon = ULCON, //Set default UART line control reg values.
		.ufcon = UFCON, //Set default UART fifo control reg values.
	},
};

#define s3c24xx_read_idcode() __raw_readl(S3C2410_GSTATUS1)

static void __init mini2440_rework_map_io(void) {
	early_printk("[GAUN] map_io\n");

	s3c24xx_init_io(mini2440_rework_iodesc,ARRAY_SIZE(mini2440_rework_iodesc));

	pr_err("mpllcon : %x\n",__raw_readl(S3C2410_MPLLCON));

	s3c24xx_init_clocks_rework(12000000); //Init the clock to 12Mhz Crystal.

	pr_crit("[GAUN] cpu architecture :%d, idcode : %x\n",
							cpu_architecture(),s3c24xx_read_idcode());


	s3c244x_init_uarts(mini2440_rework_uartcfgs,ARRAY_SIZE(mini2440_rework_uartcfgs));
}

static void __init mini2440_rework_init(void) {
	early_printk("[GAUN] machine_init\n");
}

static void __init init_mini2440_restart(char mode,const char *cmd) {
	early_printk("[GAUN] restart\n");
}

MACHINE_START(MINI2440,"MINI2440")
	.atag_offset = 0x100, //Need to check this.
	.map_io	= mini2440_rework_map_io,
	.init_machine =  mini2440_rework_init,
	.init_irq = s3c24xx_init_irq,
	.init_time = s3c24xx_timer_init,
	.restart =  init_mini2440_restart,
MACHINE_END
