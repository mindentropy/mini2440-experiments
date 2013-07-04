#ifndef CLOCK_REWORK_H_

#define CLOCK_REWORK_H_

#include <linux/clkdev.h>


struct clk {
	struct list_head list;
	struct module *owner;
	struct clk *parent;
	const char *name;
	const char *devname;
	int id;
	int usage;
	unsigned long rate;
	unsigned long ctrlbit;

	struct clk_ops *ops;

	int (*enable)(struct clk *,int enable);
	struct clk_lookup lookup;

#if defined(CONFIG_PM_DEBUG) && defined(CONFIG_DEBUG_FS)
	struct dentry *dent;
#endif
};


struct clk_ops {
	int (*set_rate)(struct clk *c,unsigned long rate);
	unsigned long (*get_rate)(struct clk *c);
	unsigned long (*round_rate)(struct clk *c,unsigned long rate);

	int (*set_parent)(struct clk *c,struct clk *parent);
};

extern int s3c24xx_register_baseclocks_rework(unsigned long xtal);

extern int s3c24xx_register_clock_rework(struct clk *clk);
extern int clk_default_set_rate_rework(struct clk *c,unsigned long rate);

/* Core clock support */
extern struct clk clk_xtal_r;
extern struct clk clk_ext_r;
extern struct clk clk_epll_r;
extern struct clk clk_mpll_r;
extern struct clk clk_upll_r;
extern struct clk clk_f_r;
extern struct clk clk_h_r;
extern struct clk clk_p_r;
extern struct clk clk_usb_bus_r;
extern struct clk s3c24xx_uclk_r;


extern void s3c_pwmclk_init(void);
#endif
