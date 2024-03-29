/* linux/arch/arm/mach-s3c2440/mach-mini2440.c
 *
 * Copyright (c) 2008 Ramax Lo <ramaxlo@gmail.com>
 *      Based on mach-anubis.c by Ben Dooks <ben@simtec.co.uk>
 *      and modifications by SBZ <sbz@spgui.org> and
 *      Weibing <http://weibing.blogbus.com> and
 *      Michel Pollet <buserror@gmail.com>
 *
 * For product information, visit http://code.google.com/p/mini2440/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/serial_core.h>
#include <linux/dm9000.h>
#include <linux/i2c/at24.h>
#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <linux/i2c.h>
#include <linux/mmc/host.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/hardware.h>
#include <mach/fb.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>
#include <linux/platform_data/leds-s3c24xx.h>
#include <mach/regs-lcd.h>
#include <mach/irqs.h>
#include <linux/platform_data/mtd-nand-s3c2410.h>
#include <linux/platform_data/i2c-s3c2410.h>
#include <linux/platform_data/mmc-s3cmci.h>
#include <linux/platform_data/usb-s3c2410_udc.h>
#include <linux/platform_data/touchscreen-s3c2410.h>
#include <linux/platform_data/hwmon-s3c.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <plat/gpio-cfg.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>

#include <sound/s3c24xx_uda134x.h>

#include "common.h"

#define MACH_MINI2440_DM9K_BASE (S3C2410_CS4 + 0x300)

static struct map_desc mini2440_iodesc[] __initdata = {
	/* nothing to declare, move along */
};

#define UCON S3C2410_UCON_DEFAULT
#define ULCON S3C2410_LCON_CS8 | S3C2410_LCON_PNONE | S3C2410_LCON_STOPB
#define UFCON S3C2410_UFCON_RXTRIG8 | S3C2410_UFCON_FIFOMODE


static struct s3c2410_uartcfg mini2440_uartcfgs[] __initdata = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.ucon	     = UCON,
		.ulcon	     = ULCON,
		.ufcon	     = UFCON,
	},
};

/* USB device UDC support */

static struct s3c2410_udc_mach_info mini2440_udc_cfg __initdata = {
	.pullup_pin = S3C2410_GPC(5),
};


/* LCD timing and setup */

/*
 * This macro simplifies the table bellow
 */
#define _LCD_DECLARE(_clock,_xres,margin_left,margin_right,hsync, \
			_yres,margin_top,margin_bottom,vsync, refresh, vwidth, vheight) \
	.width = _xres, \
	.xres = _xres, \
	.height = _yres, \
	.yres = _yres, \
	.left_margin	= margin_left,	\
	.right_margin	= margin_right,	\
	.upper_margin	= margin_top,	\
	.lower_margin	= margin_bottom,	\
	.hsync_len	= hsync,	\
	.vsync_len	= vsync,	\
	.phys_width	= vwidth,	\
	.phys_height	= vheight,	\
	.pixclock	= ((_clock*100000000000LL) /	\
			   ((refresh) * \
			   (hsync + margin_left + _xres + margin_right) * \
			   (vsync + margin_top + _yres + margin_bottom))), \
	.bpp		= 16,\
	.type		= (S3C2410_LCDCON1_TFT16BPP |\
			   S3C2410_LCDCON1_TFT)

static struct s3c2410fb_display mini2440_lcd_cfg[] __initdata = {
	[0] = {	/* mini2440 + 3.5" TFT + touchscreen (NEC NL2432HC22-23B: N35) */
		_LCD_DECLARE(
			7,			/* The 3.5 is quite fast */
			240, 21, 38, 6, 	/* x timing */
			320, 4, 4, 2,		/* y timing */
			60,			/* refresh rate */
			55, 71),		/* physical size */
		.lcdcon5	= (S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_INVVDEN |
				   S3C2410_LCDCON5_PWREN),
	},
	[1] = { /* mini2440 + 7" TFT + touchscreen (Innolux AT070TN83: N43/LCD70) */
		_LCD_DECLARE(
			10,			/* the 7" runs slower */
			800, 40, 40, 48, 	/* x timing */
			480, 29, 3, 3,		/* y timing */
			50,			/* refresh rate */
			153, 92),		/* physical size */
		.lcdcon5	= (S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_PWREN),
	},
	/* The VGA shield can outout at several resolutions. All share 
	 * the same timings, however, anything smaller than 1024x768
	 * will only be displayed in the top left corner of a 1024x768
	 * XGA output unless you add optional dip switches to the shield.
	 * Therefore timings for other resolutions have been omitted here.
	 */
	[2] = {
		_LCD_DECLARE(
			10,
			1024, 1, 2, 2,		/* y timing */
			768, 200, 16, 16, 	/* x timing */
			24,	/* refresh rate, maximum stable,
				 tested with the FPGA shield */
			0, 0),	/* Size unknown */
		.lcdcon5	= (S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_HWSWP),
	},
	[3] = {	/* mini2440 + 3.5" TFT + TS -- New model as Nov 2009 -- TD035STED4: T35 */
		_LCD_DECLARE(
			7,			/* The 3.5 is quite fast */
			240, 21, 25, 6,		/* x timing */
			320, 2, 4, 2,		/* y timing */
			40,			/* refresh rate */
			53, 71),		/* pyhsical size */
		.lcdcon5	= (S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_INVVDEN |
				   S3C2410_LCDCON5_PWREN),
	},
	[4] = { /* mini2440 + 5.6" TFT + touchscreen -- Innolux AT056TN52 */
		/* be sure the "power" jumper is set accordingly ! */
		_LCD_DECLARE(
			10,			/* the 5.3" runs slower */
			640, 41, 68, 22,	/* x timing */
			480, 26, 6, 2,		/* y timing */
			40,			/* refresh rate */
			113, 85),		/* pyhsical size */
		.lcdcon5	= (S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_PWREN),
	},
	[5] = { /* mini2440 + 3,5" TFT + touchscreen -- SONY ACX502BMU: X35 */
		_LCD_DECLARE(
			7,
			240, 1, 26, 5,		/* x timing */
			320, 1, 5, 9,		/* y timing */
			60,			/* refresh rate */
			55, 71),		/* pyhsical size */
		.lcdcon5	= (S3C2410_LCDCON5_FRM565 |
					S3C2410_LCDCON5_INVVDEN |
					S3C2410_LCDCON5_INVVFRAME |
					S3C2410_LCDCON5_INVVLINE |
					S3C2410_LCDCON5_INVVCLK |
					S3C2410_LCDCON5_HWSWP),
	},
	[6] = { /* LCD-W35i 3.5" display (Sharp LQ035Q1DG06: W35i )*/
		_LCD_DECLARE(
			/* clock */
			7,
			/* xres, margin_right, margin_left, hsync */
			320, 68, 66, 4,
			/* yres, margin_top, margin_bottom, vsync */
			240, 4, 4, 9,
			/* refresh rate */
			60,
			72, 54),	/* pyhsical size */
		.lcdcon5	= (S3C2410_LCDCON5_FRM565 |
				   S3C2410_LCDCON5_INVVDEN |
				   S3C2410_LCDCON5_INVVFRAME |
				   S3C2410_LCDCON5_INVVLINE |
				   S3C2410_LCDCON5_INVVCLK |
				   S3C2410_LCDCON5_HWSWP),
	},
	[7] = { /* mini2440 + 4.3" TFT + touchscreen (NEC NL4827HC19-01B: N43)
		   mini2440 + 4.3" TFT + touchscreen (SHARP LQ043T3DX02: N43i) */
		_LCD_DECLARE(
			9,		/* clock rate */
			480, 19, 38, 6,	/* xres, margin_right, margin_left, hsync */
			272, 1, 1, 2,	/* yres, margin_top, margin_bottom, vsync */
			60,		/* refresh rate */
			95, 54),	/* physical size */
		.lcdcon5	= (S3C2410_LCDCON5_FRM565 |
					S3C2410_LCDCON5_INVVFRAME |
					S3C2410_LCDCON5_INVVLINE |
					S3C2410_LCDCON5_PWREN |
					S3C2410_LCDCON5_HWSWP),
	},
};

/* todo - put into gpio header */

#define S3C2410_GPCCON_MASK(x)	(3 << ((x) * 2))
#define S3C2410_GPDCON_MASK(x)	(3 << ((x) * 2))

static struct s3c2410fb_mach_info mini2440_fb_info __initdata = {
	.displays	 = &mini2440_lcd_cfg[0], /* not constant! see init */
	.num_displays	 = 1,
	.default_display = 0,

	/* Enable VD[2..7], VD[10..15], VD[18..23] and VCLK, syncs, VDEN
	 * and disable the pull down resistors on pins we are using for LCD
	 * data. */

	.gpcup		= (0xf << 1) | (0x3f << 10),

	.gpccon		= (S3C2410_GPC1_VCLK   | S3C2410_GPC2_VLINE |
			   S3C2410_GPC3_VFRAME | S3C2410_GPC4_VM |
			   S3C2410_GPC10_VD2   | S3C2410_GPC11_VD3 |
			   S3C2410_GPC12_VD4   | S3C2410_GPC13_VD5 |
			   S3C2410_GPC14_VD6   | S3C2410_GPC15_VD7),

	.gpccon_mask	= (S3C2410_GPCCON_MASK(1)  | S3C2410_GPCCON_MASK(2)  |
			   S3C2410_GPCCON_MASK(3)  | S3C2410_GPCCON_MASK(4)  |
			   S3C2410_GPCCON_MASK(10) | S3C2410_GPCCON_MASK(11) |
			   S3C2410_GPCCON_MASK(12) | S3C2410_GPCCON_MASK(13) |
			   S3C2410_GPCCON_MASK(14) | S3C2410_GPCCON_MASK(15)),

	.gpdup		= (0x3f << 2) | (0x3f << 10),

	.gpdcon		= (S3C2410_GPD2_VD10  | S3C2410_GPD3_VD11 |
			   S3C2410_GPD4_VD12  | S3C2410_GPD5_VD13 |
			   S3C2410_GPD6_VD14  | S3C2410_GPD7_VD15 |
			   S3C2410_GPD10_VD18 | S3C2410_GPD11_VD19 |
			   S3C2410_GPD12_VD20 | S3C2410_GPD13_VD21 |
			   S3C2410_GPD14_VD22 | S3C2410_GPD15_VD23),

	.gpdcon_mask	= (S3C2410_GPDCON_MASK(2)  | S3C2410_GPDCON_MASK(3) |
			   S3C2410_GPDCON_MASK(4)  | S3C2410_GPDCON_MASK(5) |
			   S3C2410_GPDCON_MASK(6)  | S3C2410_GPDCON_MASK(7) |
			   S3C2410_GPDCON_MASK(10) | S3C2410_GPDCON_MASK(11)|
			   S3C2410_GPDCON_MASK(12) | S3C2410_GPDCON_MASK(13)|
			   S3C2410_GPDCON_MASK(14) | S3C2410_GPDCON_MASK(15)),
};

/* MMC/SD  */

static struct s3c24xx_mci_pdata mini2440_mmc_cfg __initdata = {
   .gpio_detect   = S3C2410_GPG(8),
   .gpio_wprotect = S3C2410_GPH(8),
   .set_power     = NULL,
   .ocr_avail     = MMC_VDD_32_33|MMC_VDD_33_34,
};

/* NAND Flash on MINI2440 board */

static struct mtd_partition mini2440_default_nand_part[] __initdata = {
	[0] = {
		.name	= "u-boot",
		.size	= SZ_256K,
		.offset	= 0,
	},
	[1] = {
		.name	= "u-boot-env",
		.size	= SZ_128K,
		.offset	= SZ_256K,
	},
	[2] = {
		.name	= "kernel",
		/* 5 megabytes, for a kernel with no modules
		 * or a uImage with a ramdisk attached */
		.size	= 0x00500000,
		.offset	= SZ_256K + SZ_128K,
	},
	[3] = {
		.name	= "root",
		.offset	= SZ_256K + SZ_128K + 0x00500000,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct s3c2410_nand_set mini2440_nand_sets[] __initdata = {
	[0] = {
		.name		= "nand",
		.nr_chips	= 1,
		.nr_partitions	= ARRAY_SIZE(mini2440_default_nand_part),
		.partitions	= mini2440_default_nand_part,
		.flash_bbt 	= 1, /* we use u-boot to create a BBT */
	},
};

static struct s3c2410_platform_nand mini2440_nand_info __initdata = {
	.tacls		= 0,
	.twrph0		= 25,
	.twrph1		= 15,
	.nr_sets	= ARRAY_SIZE(mini2440_nand_sets),
	.sets		= mini2440_nand_sets,
	.ignore_unset_ecc = 1,
};

/* DM9000AEP 10/100 ethernet controller */

static struct resource mini2440_dm9k_resource[] = {
	[0] = DEFINE_RES_MEM(MACH_MINI2440_DM9K_BASE, 4),
	[1] = DEFINE_RES_MEM(MACH_MINI2440_DM9K_BASE + 4, 4),
	[2] = DEFINE_RES_NAMED(IRQ_EINT7, 1, NULL, IORESOURCE_IRQ \
						| IORESOURCE_IRQ_HIGHEDGE),
};

/*
 * The DM9000 has no eeprom, and it's MAC address is set by
 * the bootloader before starting the kernel.
 */
static struct dm9000_plat_data mini2440_dm9k_pdata = {
	.flags		= (DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM),
};

static struct platform_device mini2440_device_eth = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mini2440_dm9k_resource),
	.resource	= mini2440_dm9k_resource,
	.dev		= {
		.platform_data	= &mini2440_dm9k_pdata,
	},
};

/*  CON5
 *	+--+	 /-----\
 *	|  |    |	|
 *	|  |	|  BAT	|
 *	|  |	 \_____/
 *	|  |
 *	|  |  +----+  +----+
 *	|  |  | K5 |  | K1 |
 *	|  |  +----+  +----+
 *	|  |  +----+  +----+
 *	|  |  | K4 |  | K2 |
 *	|  |  +----+  +----+
 *	|  |  +----+  +----+
 *	|  |  | K6 |  | K3 |
 *	|  |  +----+  +----+
 *	  .....
 */
/*static struct gpio_keys_button mini2440_buttons[] = {
	{
		.gpio		= S3C2410_GPG(0),		// K1
		.code		= KEY_F1,
		.desc		= "Button 1",
		.active_low	= 1,
	},
	{
		.gpio		= S3C2410_GPG(3),		// K2
		.code		= KEY_F2,
		.desc		= "Button 2",
		.active_low	= 1,
	},
	{
		.gpio		= S3C2410_GPG(5),		// K3
		.code		= KEY_F3,
		.desc		= "Button 3",
		.active_low	= 1,
	},
	{
		.gpio		= S3C2410_GPG(6),		// K4
		.code		= KEY_POWER,
		.desc		= "Power",
		.active_low	= 1,
	},
	{
		.gpio		= S3C2410_GPG(7),		// K5
		.code		= KEY_F5,
		.desc		= "Button 5",
		.active_low	= 1,
	},
	{
		.gpio		= S3C2410_GPG(11),		// K6
		.code		= KEY_F6,
		.desc		= "Button 6",
		.active_low	= 1,
	},
};

static struct gpio_keys_platform_data mini2440_button_data = {
	.buttons	= mini2440_buttons,
	.nbuttons	= ARRAY_SIZE(mini2440_buttons),
};

static struct platform_device mini2440_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.dev		= {
		.platform_data	= &mini2440_button_data,
	}
};
*/

static struct gpio_keys_button gaun_mini2440_buttons[] = {
	{
		.gpio =  S3C2410_GPG(0), //K1
		.active_low = 1,
		.desc = "Button 1",
		//.code = KEY_R,
		.code = KEY_SLEEP,
	},
	{
		.gpio =  S3C2410_GPG(3), //K2
		.active_low = 1,
		.desc = "Button 2",
		.code = KEY_O,
	},
	{
		.gpio =  S3C2410_GPG(5), //K3
		.active_low = 1,
		.desc = "Button 3",
		.code = KEY_T,
	},
	{
		.gpio =  S3C2410_GPG(6), //K4
		.active_low = 1,
		.desc = "Button 4",
		.code = KEY_L
	},
	{
		.gpio =  S3C2410_GPG(7), //K5
		.active_low = 1,
		.desc = "Button 5",
		.code = KEY_S
	},
	{
		.gpio =  S3C2410_GPG(11), //K6
		.active_low = 1,
		.desc = "Button 6",
		.code = KEY_ENTER
	},
};

static struct gpio_keys_platform_data gaun_mini2440_button_data= {
	.buttons = gaun_mini2440_buttons,
	.nbuttons = ARRAY_SIZE(gaun_mini2440_buttons),
};

static struct platform_device gaun_mini2440_button_device = {
	.name = "gpio-keys",
	.id = -1,
	.dev = { 
		.platform_data = &gaun_mini2440_button_data,
	}
};

/* LEDS */
/*static struct gpio_led gpio_leds[] = {
	{
		.name			= "led1",
		.gpio			= S3C2410_GPB(5),
		.active_low		= 1,
		.default_trigger	= "heartbeat",
	},
	{
		.name			= "led2",
		.gpio			= S3C2410_GPB(6),
		.active_low		= 1,
		.default_trigger	= "nand-disk",
		.default_state		= LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name			= "led3",
		.gpio			= S3C2410_GPB(7),
		.active_low		= 1,
		.default_trigger	= "mmc0",
		.default_state		= LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name			= "led4",
		.gpio			= S3C2410_GPB(8),
		.active_low		= 1,
		.default_trigger	= "none",
		.default_state		= LEDS_GPIO_DEFSTATE_OFF,
	}
};*/

static struct gpio_led backlight_led[] = {
	{
		.name			= "backlight",
		.gpio			= S3C2410_GPG(4),
		.active_low		= 0,
		.default_trigger	= "backlight",
		.default_state		= LEDS_GPIO_DEFSTATE_ON,
	}
};

/*static struct gpio_led_platform_data gpio_led_info = {
	.leds		= gpio_leds,
	.num_leds	= ARRAY_SIZE(gpio_leds),
};*/

static struct gpio_led_platform_data backlight_info = {
	.leds		= backlight_led,
	.num_leds	= ARRAY_SIZE(backlight_led),
};

/*static struct platform_device mini2440_leds = {
	.name	= "leds-gpio",
	.id	= 0,
	.dev	= {
		.platform_data	= &gpio_led_info,
	}
};*/

static struct gpio_led gaun_gpio_leds[] = {
	{
		.name = "led1",
		.default_trigger = "heartbeat",
		.active_low = 1,
		.gpio = S3C2410_GPB(5),
	},
	{
		.name = "led2",
		.default_trigger = "nand-disk",
		.active_low = 1,
		.gpio = S3C2410_GPB(6),
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name = "led3",
		.default_trigger = "mmc0",
		.active_low = 1,
		.gpio = S3C2410_GPB(7),
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
		
	},
	{
		.name = "led4",
		.default_trigger = "cpu0",
		.active_low = 1,
		.gpio = S3C2410_GPB(8),
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},

};

static struct gpio_led_platform_data gaun_gpio_led_info = {
	.leds = gaun_gpio_leds,
	.num_leds = ARRAY_SIZE(gaun_gpio_leds),
};

static struct platform_device gaun_mini2440_leds = {
	.name = "leds-gpio",
	.id = 0,
	.dev = {
		.platform_data = &gaun_gpio_led_info,
	}
};

static struct platform_device mini2440_led_backlight = {
	.name	= "leds-gpio",
	.id	= 1,
	.dev	= {
		.platform_data	= &backlight_info,
	}
};

/* AUDIO */

static struct s3c24xx_uda134x_platform_data mini2440_audio_pins = {
	.l3_clk = S3C2410_GPB(4),
	.l3_mode = S3C2410_GPB(2),
	.l3_data = S3C2410_GPB(3),
	.model = UDA134X_UDA1341
};

static struct platform_device mini2440_audio = {
	.name		= "s3c24xx_uda134x",
	.id		= 0,
	.dev		= {
		.platform_data	= &mini2440_audio_pins,
	},
};

/*
 * I2C devices
 */
/*static struct at24_platform_data at24c08 = {
	.byte_len	= SZ_8K / 8,
	.page_size	= 16,
};

static struct i2c_board_info mini2440_i2c_devs[] __initdata = {
	{
		I2C_BOARD_INFO("24c08", 0x50),
		.platform_data = &at24c08,
	},
};*/


static struct at24_platform_data gaun_at24c08 = {
	.byte_len = SZ_8K/8,
	.page_size = 16, //16 byte page
};


static struct i2c_board_info gaun_mini2440_i2c_devs[] __initdata = { 
	{
		I2C_BOARD_INFO("24c08",0x50),
		.platform_data = &gaun_at24c08,
	},
};


static struct platform_device uda1340_codec = {
		.name = "uda134x-codec",
		.id = -1,
};

static struct s3c2410_ts_mach_info mini2440_ts_cfg __initdata = {
	.delay = 10000,
	.presc = 0xff, /* slow as we can go */
	.oversampling_shift = 0,
};

static struct s3c_hwmon_chcfg mini2440_adc_chcfg[8] = {
	{
		.name = "AIN0",	/* connected to CON4/5 and W1 */
		.mult = 3300,
		.div = 1023,
	}, {
		.name = "AIN1",	/* connected to CON4/6 */
		.mult = 3300,
		.div = 1023,
	}, {
		.name = "AIN2",	/* connected to CON4/7 */
		.mult = 3300,
		.div = 1023,
	}, {
		.name = "AIN3",	/* connected to CON4/8 */
		.mult = 3300,
		.div = 1023,
	}, {
		.name = "TSYM",
		.mult = 3300,
		.div = 1023,
	}, {
		.name = "TSYP",
		.mult = 3300,
		.div = 1023,
	}, {
		.name = "TSXM",
		.mult = 3300,
		.div = 1023,
	}, {
		.name = "TSXP",
		.mult = 3300,
		.div = 1023,
	},
};

static struct s3c_hwmon_pdata mini2440_adc __initdata = {
	.in[0] = &mini2440_adc_chcfg[0],
	.in[1] = &mini2440_adc_chcfg[1],
	.in[2] = &mini2440_adc_chcfg[2],
	.in[3] = &mini2440_adc_chcfg[3],
	.in[4] = &mini2440_adc_chcfg[4],
	.in[5] = &mini2440_adc_chcfg[5],
	.in[6] = &mini2440_adc_chcfg[6],
	.in[7] = &mini2440_adc_chcfg[7],
};

static struct platform_device mini2440_buzzer_device = {
	.name = "pwm-beeper",
	.id = -1,
	.dev = {
		.parent = &s3c_device_timer[0].dev,
		.platform_data = (void *)0,	/* channel 0 */
	},
};

static struct platform_device *mini2440_devices[] __initdata = {
	&s3c_device_ohci,
	&s3c_device_wdt,
	&s3c_device_i2c0,
	&s3c_device_rtc,
	&s3c_device_usbgadget,
	&mini2440_device_eth,
	&gaun_mini2440_leds,
	&gaun_mini2440_button_device,
	&s3c_device_nand,
	&s3c_device_sdi,
	&s3c_device_iis,
	&uda1340_codec,
	&mini2440_audio,
	&s3c_device_adc,
	&s3c_device_hwmon,
	&s3c_device_timer[0],
	&mini2440_buzzer_device,
};


static void __init mini2440_map_io(void)
{
	printk(KERN_CRIT "[GAUN] map_io\n");

	s3c24xx_init_io(mini2440_iodesc, ARRAY_SIZE(mini2440_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(mini2440_uartcfgs, ARRAY_SIZE(mini2440_uartcfgs));
}

/*
static void __init mini2440_map_io(void)
{
	s3c24xx_init_io(mini2440_iodesc, ARRAY_SIZE(mini2440_iodesc));
	s3c24xx_init_clocks(12000000);
	s3c24xx_init_uarts(mini2440_uartcfgs, ARRAY_SIZE(mini2440_uartcfgs));
}
*/

/*
 * mini2440_features string
 *
 * t = Touchscreen present
 * b = backlight control
 * c = camera [TODO]
 * 0-9 LCD configuration
 *
 */
static char mini2440_features_str[12] __initdata = "0tb";

static int __init mini2440_features_setup(char *str)
{
	if (str)
		strlcpy(mini2440_features_str, str, sizeof(mini2440_features_str));
	return 1;
}

__setup("mini2440=", mini2440_features_setup);

#define FEATURE_SCREEN (1 << 0)
#define FEATURE_BACKLIGHT (1 << 1)
#define FEATURE_TOUCH (1 << 2)
#define FEATURE_CAMERA (1 << 3)

struct mini2440_features_t {
	int count;
	int done;
	int lcd_index;
	struct platform_device *optional[8];
};

static void __init mini2440_parse_features(
		struct mini2440_features_t * features,
		const char * features_str )
{
	const char * fp = features_str;

	features->count = 0;
	features->done = 0;
	features->lcd_index = -1;

	while (*fp) {
		char f = *fp++;

		switch (f) {
		case '0'...'9':	/* tft screen */
			if (features->done & FEATURE_SCREEN) {
				pr_info("MINI2440: '%c' ignored, "
					"screen type already set\n", f);
			} else {
				int li = f - '0';
				if (li >= ARRAY_SIZE(mini2440_lcd_cfg))
					pr_info("MINI2440: "
						"'%c' out of range LCD mode\n", f);
				else {
					features->optional[features->count++] =
							&s3c_device_lcd;
					features->lcd_index = li;
				}
			}
			features->done |= FEATURE_SCREEN;
			break;
		case 'b':
			if (features->done & FEATURE_BACKLIGHT)
				pr_info("MINI2440: '%c' ignored, "
					"backlight already set\n", f);
			else {
				features->optional[features->count++] =
						&mini2440_led_backlight;
			}
			features->done |= FEATURE_BACKLIGHT;
			break;
		case 't':
			if (features->done & FEATURE_TOUCH)
				pr_info("MINI2440: '%c' ignored, "
					"touchscreen already set\n", f);
			else
				features->optional[features->count++] =
						&s3c_device_ts;
			features->done |= FEATURE_TOUCH;
			break;
		case 'c':
			if (features->done & FEATURE_CAMERA)
				pr_info("MINI2440: '%c' ignored, "
					"camera already registered\n", f);
			else
				features->optional[features->count++] =
					&s3c_device_camif;
			features->done |= FEATURE_CAMERA;
			break;
		}
	}
}

static void __init mini2440_init(void)
{
	struct mini2440_features_t features = { 0 };
	int i;
	
	pr_info("MINI2440: Option string mini2440=%s\n",
			mini2440_features_str);

	/* Parse the feature string */
	mini2440_parse_features(&features, mini2440_features_str);

	/* turn LCD on */
	s3c_gpio_cfgpin(S3C2410_GPC(0), S3C2410_GPC0_LEND);

	/* Turn the backlight early on */
	WARN_ON(gpio_request_one(S3C2410_GPG(4), GPIOF_OUT_INIT_HIGH, NULL));
	gpio_free(S3C2410_GPG(4));

	/* remove pullup on optional PWM backlight -- unused on 3.5 and 7"s */
	gpio_request_one(S3C2410_GPB(1), GPIOF_IN, NULL);
	s3c_gpio_setpull(S3C2410_GPB(1), S3C_GPIO_PULL_UP);
	gpio_free(S3C2410_GPB(1));

	/* mark the key as input, without pullups (there is one on the board) */

	for(i = 0;i<ARRAY_SIZE(gaun_mini2440_buttons);i++) {
		s3c_gpio_setpull(gaun_mini2440_buttons[i].gpio,S3C_GPIO_PULL_UP);
		s3c_gpio_cfgpin(gaun_mini2440_buttons[i].gpio,S3C2410_GPIO_INPUT);
	}
		
/*	for (i = 0; i < ARRAY_SIZE(mini2440_buttons); i++) {
		s3c_gpio_setpull(mini2440_buttons[i].gpio, S3C_GPIO_PULL_UP);
		s3c_gpio_cfgpin(mini2440_buttons[i].gpio, S3C2410_GPIO_INPUT);
	}*/

	if (features.lcd_index != -1) {
		int li;

		mini2440_fb_info.displays =
			&mini2440_lcd_cfg[features.lcd_index];

		pr_info("MINI2440: LCD");
		for (li = 0; li < ARRAY_SIZE(mini2440_lcd_cfg); li++)
			if (li == features.lcd_index)
				pr_info(" [%d:%dx%d]", li,
					mini2440_lcd_cfg[li].width,
					mini2440_lcd_cfg[li].height);
			else
				pr_info(" %d:%dx%d", li,
					mini2440_lcd_cfg[li].width,
					mini2440_lcd_cfg[li].height);
		pr_info("\n");
		s3c24xx_fb_set_platdata(&mini2440_fb_info);
	}

	s3c24xx_udc_set_platdata(&mini2440_udc_cfg);
	s3c24xx_mci_set_platdata(&mini2440_mmc_cfg);
	s3c_nand_set_platdata(&mini2440_nand_info);
	s3c_i2c0_set_platdata(NULL);
	s3c24xx_ts_set_platdata(&mini2440_ts_cfg);
	s3c_hwmon_set_platdata(&mini2440_adc);

/*	i2c_register_board_info(0, mini2440_i2c_devs,
				ARRAY_SIZE(mini2440_i2c_devs));*/


	i2c_register_board_info(0,gaun_mini2440_i2c_devs,
				ARRAY_SIZE(gaun_mini2440_i2c_devs));

	/* PWM to the buzzer */
	s3c_gpio_cfgpin(S3C2410_GPB(0), S3C2410_GPB0_TOUT0);

	platform_add_devices(mini2440_devices, ARRAY_SIZE(mini2440_devices));

	if (features.count)	/* the optional features */
		platform_add_devices(features.optional, features.count);

}


MACHINE_START(MINI2440, "MINI2440")
	/* Maintainer: Michel Pollet <buserror@gmail.com> */
	.atag_offset	= 0x100,
	.map_io		= mini2440_map_io,
	.init_machine	= mini2440_init,
	.init_irq	= s3c24xx_init_irq,
	.init_time	= s3c24xx_timer_init,
	.restart	= s3c244x_restart,
MACHINE_END
