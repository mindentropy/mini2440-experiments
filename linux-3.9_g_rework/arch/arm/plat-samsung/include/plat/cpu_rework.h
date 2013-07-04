#ifndef CPU_REWORK_H_

#define CPU_REWORK_H_

/*Reworked core initialization functions */

extern void s3c_init_cpu_rework(unsigned long idcode,
			struct cpu_table *cpus,
			unsigned int cputab_size);

extern void __init s3c24xx_init_clocks_rework(int xtal);

#endif
