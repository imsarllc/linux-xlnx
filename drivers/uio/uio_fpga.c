#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/uio_driver.h>
#include <linux/irq.h>

MODULE_LICENSE("GPL");

#define IRQ_NUM      90
#define DRIVER_NAME  "uio_fpga"

static int intrpt_cnt;

static struct resource uio_resource[] = {
	{
		.start = 0x40410000,
		.end   = 0x4041ffff,
		.name = "axi_dma_regs",
		.flags = IORESOURCE_MEM
	},
};


static irqreturn_t irq_handler(int irq, struct uio_info *dev_info) {
	switch(irq)
	{
		case IRQ_NUM:
			intrpt_cnt++;
			//printk("intrpt_cnt %d\n", intrpt_cnt);
			return IRQ_HANDLED;
	}

	return IRQ_NONE;
}


static struct uio_info myfpga_uio_info = {
	.name    = DRIVER_NAME,
	.version = "0.1",
	.irq     = IRQ_NUM,
	.handler = irq_handler,
};


static struct platform_device uio_device = { 
	.name              = "uio_pdrv",
	.id                = -1,
	.resource          = uio_resource,
	.num_resources     = 0/*ARRAY_SIZE(uio_resource)*/,
	.dev.platform_data = &myfpga_uio_info,
};


static int __init mod_init(void)
{
	int ret;

	intrpt_cnt = 0;

	ret = platform_device_register(&uio_device);
	irq_set_irq_type(IRQ_NUM, IRQ_TYPE_EDGE_RISING);

	printk(KERN_INFO "UIO module loaded for IRQ#%d (%d)\n", IRQ_NUM, ret);

	return 0;
}

static void __exit mod_exit(void)
{
	platform_device_unregister(&uio_device);
}

module_init(mod_init);
module_exit(mod_exit);
