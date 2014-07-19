#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/ssr_plug.h>


static ssize_t set_ssr_power(struct device *dev,
                               struct device_attribute *attr,
                               const char *buf, size_t count, int chan)
{
    struct ssr_plug_config *sdev = dev->platform_data;
    unsigned long power;
    int success = strict_strtoul(buf, 10, &power);

    if(!sdev) {
        printk(KERN_INFO SSR_PLUG_DRIVER_NAME ": no sdev!!\n");

        return -1;
    }

    if (success == 0) {
        if(power <= 100) {
            sdev->ssr_plug_config[chan].power = power;
            return count;
        }
        else
            return -1;
    }
    return success;
}

static ssize_t show_ssr_power(struct device *dev,
                                struct device_attribute *attr, char *buf, int chan)
{
   struct ssr_plug_config *sdev = dev->platform_data;

   if(!sdev)
       return -1;

   return sprintf(buf, "%u\n", sdev->ssr_plug_config[chan].power);
}

static ssize_t set_ssr_power1(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return set_ssr_power(dev, attr, buf, count, 0);
}

static ssize_t show_ssr_power1(struct device *dev,struct device_attribute *attr, char *buf)
{
    return show_ssr_power(dev, attr, buf, 0);
}

static ssize_t set_ssr_power2(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return set_ssr_power(dev, attr, buf, count, 1);
}

static ssize_t show_ssr_power2(struct device *dev,struct device_attribute *attr, char *buf)
{
    return show_ssr_power(dev, attr, buf, 1);
}

static ssize_t set_ssr_power3(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return set_ssr_power(dev, attr, buf, count, 2);
}

static ssize_t show_ssr_power3(struct device *dev,struct device_attribute *attr, char *buf)
{
    return show_ssr_power(dev, attr, buf, 2);
}

static ssize_t set_ssr_power4(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return set_ssr_power(dev, attr, buf, count, 3);
}

static ssize_t show_ssr_power4(struct device *dev,struct device_attribute *attr, char *buf)
{
    return show_ssr_power(dev, attr, buf, 3);
}

static DEVICE_ATTR(ssr_1, S_IWUSR | S_IRUGO,
                    show_ssr_power1, set_ssr_power1);
static DEVICE_ATTR(ssr_2, S_IWUSR | S_IRUGO,
                    show_ssr_power2, set_ssr_power2);
static DEVICE_ATTR(ssr_3, S_IWUSR | S_IRUGO,
                    show_ssr_power3, set_ssr_power3);
static DEVICE_ATTR(ssr_4, S_IWUSR | S_IRUGO,
                    show_ssr_power4, set_ssr_power4);


static struct attribute *ssr_attributes[] = {
    &dev_attr_ssr_1.attr,
    &dev_attr_ssr_2.attr,
    &dev_attr_ssr_3.attr,
    &dev_attr_ssr_4.attr,
    NULL
};

static const struct attribute_group ssr_attr_group = {
    .attrs = ssr_attributes,
};

static int ssr_plug_probe(struct platform_device *pdev)
{
    int i,ret;
    struct ssr_plug_config *sdev = pdev->dev.platform_data;

    if(!sdev)
    {
        printk(KERN_ERR SSR_PLUG_DRIVER_NAME": platform_get_drvdata in probe failed\n");
        return -1;
    }
    spin_lock_init(&sdev->ssr_lock);

    for(i=0;i<sdev->plugCnt;i++) {
        ret = gpio_request(sdev->ssr_plug_config[i].gpio, "ssr_plug");
        if (ret)
            pr_warning("Can't req gpio %d for ssr_plug\n", sdev->ssr_plug_config[i].gpio);

        gpio_direction_output(sdev->ssr_plug_config[i].gpio, 0);
    }
    sdev->initialized = 1;
    platform_set_drvdata(pdev, sdev);

    ret = sysfs_create_group(&pdev->dev.kobj, &ssr_attr_group);
    if (ret)
        goto exit_free;

    printk(KERN_INFO SSR_PLUG_DRIVER_NAME": probed\n");
    return 0;

exit_free:
    platform_set_drvdata(pdev, NULL);

    for(i=0;i<sdev->plugCnt;i++) {
        gpio_free(sdev->ssr_plug_config[i].gpio);
    }
    return ret;
}

static int ssr_plug_remove(struct platform_device *pdev)
{
    int i;
    struct ssr_plug_config *sdev = platform_get_drvdata(pdev);

    if (!sdev)
        return -ENOENT;

    platform_set_drvdata(pdev, NULL);

    for(i=0;i<sdev->plugCnt;i++) {
        gpio_free(sdev->ssr_plug_config[i].gpio);
    }
    printk(KERN_INFO SSR_PLUG_DRIVER_NAME": ssrplug removed\n");

    return 0;
}

static struct platform_driver ssr_plug_driver = {
    .probe		= ssr_plug_probe,
    .driver = {
        .name   = SSR_PLUG_DRIVER_NAME,
        .owner  = THIS_MODULE,
    },
};

static int __init ssr_plug_init(void)
{
	int result;

	result = platform_driver_register(&ssr_plug_driver);
	if (result) {
		printk(KERN_ERR SSR_PLUG_DRIVER_NAME
		       ": lirc register returned %d\n", result);
		return result;
	}
    return 0;
}

static void ssr_plug_exit(void)
{
	platform_driver_unregister(&ssr_plug_driver);
}

static int __init ssr_plug_init_module(void)
{
    int result;

	result = ssr_plug_init();
	if (result)
		return result;

	printk(KERN_INFO SSR_PLUG_DRIVER_NAME ": driver registered!\n");

	return 0;
}

static void __exit ssr_plug_exit_module(void)
{
	ssr_plug_exit();

	printk(KERN_INFO SSR_PLUG_DRIVER_NAME ": cleaned up module\n");
}

module_init(ssr_plug_init_module);
//module_exit(ssr_plug_exit_module);

MODULE_DESCRIPTION("blahblah");
MODULE_AUTHOR("Sebastian Duell <sebastian.duell@gmx.de>");
MODULE_LICENSE("GPL");
