/*
 * drivers/misc/timed_output.c
 *
 * Copyright (C) 2009 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/timed_output.h>
#include <linux/slab.h>       /* for kzalloc(), etc. */

static struct class *timed_output_class;
static atomic_t device_count;

static ssize_t enable_show(struct device *dev, struct device_attribute *attr,
                           char *buf)
{
    struct timed_output_dev *tdev = dev_get_drvdata(dev);
    int remaining = tdev->get_time(tdev);

    return sprintf(buf, "%d\n", remaining);
}

static ssize_t enable_store(struct device *dev, struct device_attribute *attr,
                            const char *buf, size_t size)
{
    struct timed_output_dev *tdev = dev_get_drvdata(dev);
    int value;

    sscanf(buf, "%d", &value);
    tdev->enable(tdev, value);

    return size;
}

static DEVICE_ATTR(enable, 0644 /* S_IRUGO|S_IWUSR */, enable_show, enable_store);

static int create_timed_output_class(void)
{
    if (!timed_output_class) {
        timed_output_class = class_create(THIS_MODULE, "timed_output");
        if (IS_ERR(timed_output_class))
            return PTR_ERR(timed_output_class);

        atomic_set(&device_count, 0);
    }
    return 0;
}

int timed_output_dev_register(struct timed_output_dev *tdev)
{
    int ret;

    if (!tdev || !tdev->name || !tdev->enable || !tdev->get_time)
        return -EINVAL;

    ret = create_timed_output_class();
    if (ret < 0)
        return ret;

    /* get a unique minor based on device_count */
    tdev->index = atomic_inc_return(&device_count);

    /* Create a device in /sys/class/timed_output/<tdev->name> */
    tdev->dev = device_create(timed_output_class, NULL,
                              MKDEV(0, tdev->index),
                              NULL, /* no private drvdata here */
                              "%s", tdev->name);
    if (IS_ERR(tdev->dev)) {
        ret = PTR_ERR(tdev->dev);
        pr_err("timed_output: Failed device_create for %s\n", tdev->name);
        return ret;
    }

    /* Associate the timed_output_dev struct with this struct device: */
    dev_set_drvdata(tdev->dev, tdev);

    /* Create the "enable" sysfs file */
    ret = device_create_file(tdev->dev, &dev_attr_enable);
    if (ret < 0) {
        pr_err("timed_output: Failed to create sysfs 'enable' for %s\n",
               tdev->name);
        device_destroy(timed_output_class, MKDEV(0, tdev->index));
        return ret;
    }

    tdev->state = 0;
    return 0;
}
EXPORT_SYMBOL_GPL(timed_output_dev_register);

void timed_output_dev_unregister(struct timed_output_dev *tdev)
{
    if (!tdev || !tdev->dev)
        return;

    device_remove_file(tdev->dev, &dev_attr_enable);
    device_destroy(timed_output_class, MKDEV(0, tdev->index));
    dev_set_drvdata(tdev->dev, NULL);
}
EXPORT_SYMBOL_GPL(timed_output_dev_unregister);

static int __init timed_output_init(void)
{
    return create_timed_output_class();
}

static void __exit timed_output_exit(void)
{
    if (timed_output_class) {
        class_destroy(timed_output_class);
        timed_output_class = NULL;
    }
}

module_init(timed_output_init);
module_exit(timed_output_exit);

MODULE_AUTHOR("Mike Lockwood <lockwood@android.com>");
MODULE_DESCRIPTION("timed output class driver");
MODULE_LICENSE("GPL");
