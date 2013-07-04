#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <plat/clock_rework.h>
#include <plat/cpu.h>


#if defined(CONFIG_DEBUG_FS)
#include <linux/debugfs.h>
#endif

int clk_default_set_rate_rework(struct clk *clk, unsigned long rate)
{
	clk->rate = rate;
	return 0;
}

struct clk_ops clk_ops_def_setrate_r = {
	.set_rate = clk_default_set_rate_rework,
};

struct clk clk_xtal_r = {  //Main external crystal.
	.name = "xtal",
	.rate = 0,
	.parent = NULL,
	.ctrlbit = 0,
};


struct clk clk_ext_r = { //External clock
	.name = "ext",
};

struct clk clk_epll_r = { //External pll. TODO:Is this needed?
	.name = "epll",
};

struct clk clk_mpll_r = {
	.name = "mpll",
};

struct clk clk_upll_r = {
	.name = "upll",
};

struct clk clk_f_r = {
	.name = "fclk",
	.rate = 0,
	.parent = &clk_mpll_r,
	.ctrlbit = 0,
};

struct clk clk_h_r = {
	.name = "hclk",
	.rate = 0,
	.parent = NULL,
	.ctrlbit = 0,
	.ops = &clk_ops_def_setrate_r,
};

struct clk clk_p_r = {
	.name = "pclk",
	.rate = 0,
	.parent = NULL,
	.ctrlbit = 0,
	.ops = &clk_ops_def_setrate_r,
};

struct clk clk_usb_bus_r = {
	.name = "usb-bus",
	.rate = 0,
	.parent = &clk_upll_r,
};

struct clk s3c24xx_uclk_r = {
	.name = "uclk",
};


static int clk_null_enable_r(struct clk *clk, int enable)
{
	return 0;
}


/* Add the clk device */
int s3c24xx_register_clock_rework(struct clk *clk)
{
	if(clk->enable == NULL)
		clk->enable = clk_null_enable_r;

	/* Fill up the clk_lookup structure and register it */
	clk->lookup.dev_id = clk->devname; //TODO: Never created device name!! Shouldn't it be init to NULL?
	clk->lookup.con_id = clk->name;
	clk->lookup.clk = clk;

	clkdev_add(&clk->lookup);

	return 0;
}

/* Register an array of clock pointers */
int s3c24xx_register_clocks_rework(struct clk **clks, int nr_clks)
{
	int fails = 0;
	
	for(;nr_clks>0;nr_clks--,clks++) {
		if(s3c24xx_register_clock_rework(*clks) < 0) { //Never returns < 0. Hence will always pass.
			struct clk *clk = *clks;
			printk(KERN_ERR "%s: failed to register %p: %s\n",
								__func__,
								clk,
								clk->name);
									
			fails++;
		}
	}

	return fails;
}

int __init s3c24xx_register_baseclocks_rework(unsigned long xtal)
{
	printk(KERN_INFO "S324XXX clks, [GAUN]\n");

	clk_xtal_r.rate = xtal;

	if(s3c24xx_register_clock_rework(&clk_xtal_r) < 0) {
		printk(KERN_ERR "Failed to register master xtal\n");
	}

	if(s3c24xx_register_clock_rework(&clk_mpll_r) < 0) {
		printk(KERN_ERR "Failed to register mpll clock\n");
	}

	if(s3c24xx_register_clock_rework(&clk_upll_r) < 0) {
		printk(KERN_ERR "Failed to register upll clock\n");
	}

	if(s3c24xx_register_clock_rework(&clk_f_r) < 0) {
		printk(KERN_ERR "Failed to register cpu fclk\n");
	}

	if(s3c24xx_register_clock_rework(&clk_h_r) < 0) {
		printk(KERN_ERR "Failed to register cpu hclk\n");
	}

	if(s3c24xx_register_clock_rework(&clk_p_r) < 0) {
		printk(KERN_ERR "Failed to register cpu pclk\n");
	}

	return 0;
}
