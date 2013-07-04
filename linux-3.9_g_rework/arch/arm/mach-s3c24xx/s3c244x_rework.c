#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <mach/regs-clock.h>
#include <plat/clock_rework.h>
#include <plat/pll.h>
#include <plat/cpu.h>
#include <plat/cpu-freq.h>
#include <plat/clock_rework.h>

static inline unsigned int s3c2440_get_pll(
						unsigned int pllval,
						unsigned int baseclk)
{
	unsigned int mdiv,pdiv,sdiv;
	uint64_t fvco;

	mdiv = (pllval >> S3C24XX_PLL_MDIV_SHIFT) & S3C24XX_PLL_MDIV_MASK;
	pdiv = (pllval >> S3C24XX_PLL_PDIV_SHIFT) & S3C24XX_PLL_PDIV_MASK;
	sdiv = (pllval >> S3C24XX_PLL_SDIV_SHIFT) & S3C24XX_PLL_SDIV_MASK;

	/*
	 * MPLL -> Fout = (2 * m * Fin)/(p * 2^s) 
	 *
	 * m = MDIV + 8, p = PDIV + 2,s = SDIV
	 */
	fvco = (uint64_t)baseclk * (mdiv + 8);
	do_div(fvco,(pdiv + 2) << sdiv);

	return (unsigned int)fvco;
}

void __init_or_cpufreq s3c24xx_setup_clocks_rework(void)
{
	struct clk *xtal_clk;

	unsigned long clkdiv;
	unsigned long camdiv;
	unsigned long xtal;

	unsigned long hclk,fclk,pclk;
	int hdiv = 1;

	xtal_clk = clk_get(NULL,"xtal");
	xtal = clk_get_rate(xtal_clk);
	clk_put(xtal_clk);

	/*
	 * MPLL -> Fout = (2 * m * Fin)/(p * 2^s) 
	 *
	 * m = MDIV + 8, p = PDIV + 2,s = SDIV
	 *
	 * The 2 is multiplied here.
	 */
	fclk = s3c24xx_get_pll(__raw_readl(S3C2410_MPLLCON),xtal) * 2;

	clkdiv = __raw_readl(S3C2410_CLKDIVN);
	camdiv = __raw_readl(S3C2440_CAMDIVN);

	switch(clkdiv & S3C2440_CLKDIVN_HDIVN_MASK) {
		case S3C2440_CLKDIVN_HDIVN_1:
			hdiv = 1;
			break;
		case S3C2440_CLKDIVN_HDIVN_2:
			hdiv = 2;
			break;
		case S3C2440_CLKDIVN_HDIVN_4_8:
			if(camdiv & S3C2440_CAMDIVN_HCLK4_HALF) //If 9th bit is set.
				hdiv = 8;
			else
				hdiv = 4;

			break;
		case S3C2440_CLKDIVN_HDIVN_3_6:
			if(camdiv & S3C2440_CAMDIVN_HCLK3_HALF)
				hdiv = 6;
			else
				hdiv = 3;

			break;
	}
	
	
	hclk = fclk / hdiv;
	pclk = hclk / ((clkdiv & S3C2440_CLKDIVN_PDIVN) ? 2 : 1);

/*	pr_err("mpllcon : %x\n",pllval = __raw_readl(S3C2410_MPLLCON));
	pr_err("upllcon : %x\n",__raw_readl(S3C2410_UPLLCON));*/

	//fclk = s3c24xx_get_pll(__raw_readl(S3C2410_MPLLCON),xtal)*2;

/*	printk(KERN_CRIT "mdiv:%x, pdiv:%x, sdiv:%x\n",
			(pllval >> S3C24XX_PLL_MDIV_SHIFT) & S3C24XX_PLL_MDIV_MASK,
			(pllval >> S3C24XX_PLL_PDIV_SHIFT) & S3C24XX_PLL_PDIV_MASK,
			(pllval >> S3C24XX_PLL_SDIV_SHIFT) & S3C24XX_PLL_SDIV_MASK);*/

	printk(KERN_CRIT "S3C244X: core %ld.%03ld MHz, memory %ld.%03ld MHz, peripheral %ld.%03ld MHz\n",
	       print_mhz(fclk), print_mhz(hclk), print_mhz(pclk));
	
	clk_mpll_r.rate = fclk;
	clk_f_r.rate = fclk;

	clk_h_r.rate = hclk;
	clk_p_r.rate = pclk;
}

int s3c2410_clkcon_enable_r(struct clk *clk,int enable)
{
	return 0;
}

static struct clk init_clocks[] = {
	{
		.name = "lcd",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_LCDC,
	}, {
		.name = "gpio",
		.parent = &clk_p_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_GPIO,
	}, {
		.name = "usb-host",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_USBH,
	}, {
		.name = "usb-device",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_USBD,
	}, {
		.name = "timers",
		.parent = &clk_p_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_PWMT,
	}, {
		.name = "uart",
		.devname = "s32410-uart.0",
		.parent = &clk_p_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_UART0,
	}, {
		.name = "uart",
		.devname = "s32410-uart.1",
		.parent = &clk_p_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_UART1,
	}, {
		.name = "uart",
		.devname = "s32410-uart.2",
		.parent = &clk_p_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_UART2,
	}, {
		.name = "rtc",
		.parent = &clk_p_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_RTC,
	}, {
		.name = "watchdog",
		.parent = &clk_p_r,
		.ctrlbit = 0,
	}, {
		.name = "usb-bus-host",
		.parent = &clk_usb_bus_r,
	}, {
		.name = "usb-bus-gadget",
		.parent = &clk_usb_bus_r,
	},
};

static struct clk init_clocks_off[] = {
	{
		.name = "nand",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_NAND,
	}, 
	{
		.name = "sdi",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_SDI,
	},
	{
		.name = "adc",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_ADC,
	},
	{
		.name = "i2c",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_IIC,
	},
	{
		.name = "iis",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_IIS,
	},
	{
		.name = "spi",
		.parent = &clk_h_r,
		.enable = s3c2410_clkcon_enable_r,
		.ctrlbit = S3C2410_CLKCON_SPI,
	},
};


static int s3c2440_upll_enable(struct clk *clk, int enable)
{
	unsigned long clkslow = __raw_readl(S3C2410_CLKSLOW);
	unsigned long orig = clkslow;

	if(enable) {
		clkslow &= ~S3C2410_CLKSLOW_UCLK_OFF;
	} else {
		clkslow |= S3C2410_CLKSLOW_UCLK_OFF;
	}

	__raw_writel(clkslow,S3C2410_CLKSLOW);

	if(enable && (orig & S3C2410_CLKSLOW_UCLK_OFF))
		udelay(200);

	return 0;
}

void __init s3c2440_reg_clocks(struct clk *clkp,int nr_clks) 
{
	int ret;

	for(;nr_clks > 0; nr_clks--,clkp++) {
		ret = s3c24xx_register_clock_rework(clkp);
		if(ret < 0) {
			printk(KERN_ERR "Failed to register clock %s (%d)\n",clkp->name,ret);
		}
	}
}

void __init s3c2440_disable_clocks(struct clk *clkp,
								int nr_clks) {
	for(;nr_clks > 0; nr_clks--,clkp++) {
		clkp->enable(clkp,0);
	}
}

int __init s3c2440_baseclk_add_rework(void) {
	unsigned long clkslow = __raw_readl(S3C2410_CLKSLOW);
	unsigned long clkcon  = __raw_readl(S3C2410_CLKCON);

	struct clk *clkp;
	struct clk *xtal;

	int ret;
	int ptr;

	clk_upll_r.enable = s3c2440_upll_enable;
	
	if(s3c24xx_register_clock_rework(&clk_usb_bus_r) < 0)
		printk(KERN_ERR "Failed to register usb bus clock\n");
	
	clkp = init_clocks;

	for(ptr = 0; ptr<ARRAY_SIZE(init_clocks); ptr++,clkp++) {
		if(clkcon & clkp->ctrlbit) {
			clkp->usage = 1;
		} else {
			clkp->usage = 0;
		}
		ret = s3c24xx_register_clock_rework(clkp);

		if(ret < 0) {
			printk(KERN_CRIT "Failed to register clock %s (%d)\n",
							clkp->name,ret);
		}
	}
	
	s3c2440_reg_clocks(init_clocks_off,ARRAY_SIZE(init_clocks_off));
	s3c2440_disable_clocks(init_clocks_off,ARRAY_SIZE(init_clocks_off));


	printk(KERN_CRIT "clkslow: %lx, clkcon: %lx\n",clkslow,clkcon);

	xtal = clk_get(NULL, "xtal");

	printk(KERN_INFO "CLOCK: Slow mode (%ld.%ld MHz), %s, MPLL %s, UPLL %s\n",
	       print_mhz(clk_get_rate(xtal) /
			 ( 2 * S3C2410_CLKSLOW_GET_SLOWVAL(clkslow))),
	       (clkslow & S3C2410_CLKSLOW_SLOW) ? "slow" : "fast",
	       (clkslow & S3C2410_CLKSLOW_MPLL_OFF) ? "off" : "on",
	       (clkslow & S3C2410_CLKSLOW_UCLK_OFF) ? "off" : "on");

	s3c_pwmclk_init();

	return 0;

}

/* 
 * This function address is set in function pointer in mach-s3c244x/common.c 
 * cpu id table.
 */
void __init s3c244x_init_clocks_rework(int xtal)
{
	printk(KERN_ERR "Clock init not implemented\n");


	s3c24xx_register_baseclocks_rework(xtal);
	s3c24xx_setup_clocks_rework();
	s3c2440_baseclk_add_rework();

	while(1)
		;
}
