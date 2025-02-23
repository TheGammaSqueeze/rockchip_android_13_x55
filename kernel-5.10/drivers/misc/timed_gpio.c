/*
 * drivers/misc/timed_gpio.c
 *
 * Copyright (C) 2008 Google, Inc.
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
#include <linux/platform_device.h>
#include <linux/hrtimer.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>
#include <linux/timed_output.h>
#include <linux/timed_gpio.h>
#include <linux/slab.h>     /* for kzalloc() */

struct timed_gpio_data {
    struct timed_output_dev dev;
    struct hrtimer timer;
    spinlock_t lock;
    unsigned gpio;
    int max_timeout;
    u8 active_low;
};

static enum hrtimer_restart gpio_timer_func(struct hrtimer *timer)
{
    struct timed_gpio_data *data =
        container_of(timer, struct timed_gpio_data, timer);

    /* Time expired, set GPIO to "off" */
    gpio_direction_output(data->gpio, data->active_low ? 1 : 0);
    return HRTIMER_NORESTART;
}

static int gpio_get_time(struct timed_output_dev *dev)
{
    struct timed_gpio_data *data =
        container_of(dev, struct timed_gpio_data, dev);

    if (hrtimer_active(&data->timer)) {
        ktime_t r = hrtimer_get_remaining(&data->timer);
        return (int) ktime_to_ms(r);
    } else {
        return 0;
    }
}

static void gpio_enable(struct timed_output_dev *dev, int value)
{
    struct timed_gpio_data *data =
        container_of(dev, struct timed_gpio_data, dev);
    unsigned long flags;

    spin_lock_irqsave(&data->lock, flags);

    /* cancel any previous timer */
    hrtimer_cancel(&data->timer);

    /* Turn GPIO on or off immediately */
    gpio_direction_output(data->gpio,
                          data->active_low ? !value : !!value);

    /* if value>0, arm a new timer to set GPIO back off */
    if (value > 0) {
        if (value > data->max_timeout)
            value = data->max_timeout;

        hrtimer_start(&data->timer,
                      ktime_set(value / 1000,
                                (value % 1000) * 1000000),
                      HRTIMER_MODE_REL);
    }

    spin_unlock_irqrestore(&data->lock, flags);
}

static int timed_gpio_probe(struct platform_device *pdev)
{
    struct timed_gpio_platform_data *pdata = pdev->dev.platform_data;
    struct timed_gpio *cur_gpio;
    struct timed_gpio_data *gpio_data, *gpio_dat;
    int i, ret = 0;

    if (!pdata)
        return -EBUSY;

    gpio_data = kzalloc(sizeof(struct timed_gpio_data) * pdata->num_gpios,
                        GFP_KERNEL);
    if (!gpio_data)
        return -ENOMEM;

    for (i = 0; i < pdata->num_gpios; i++) {
        cur_gpio = &pdata->gpios[i];
        gpio_dat = &gpio_data[i];

        hrtimer_init(&gpio_dat->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        gpio_dat->timer.function = gpio_timer_func;
        spin_lock_init(&gpio_dat->lock);

        /* fill in timed_output_dev fields */
        gpio_dat->dev.name     = cur_gpio->name;
        gpio_dat->dev.get_time = gpio_get_time;
        gpio_dat->dev.enable   = gpio_enable;

        /* register timed_output_dev */
        ret = timed_output_dev_register(&gpio_dat->dev);
        if (ret < 0) {
            int k;
            /* unwind previously registered devs */
            for (k = 0; k < i; k++)
                timed_output_dev_unregister(&gpio_data[k].dev);

            kfree(gpio_data);
            return ret;
        }

        gpio_dat->gpio = cur_gpio->gpio;
        gpio_dat->max_timeout = cur_gpio->max_timeout;
        gpio_dat->active_low  = cur_gpio->active_low;

        /* set default to 'off' */
        gpio_direction_output(gpio_dat->gpio, gpio_dat->active_low);
    }

    platform_set_drvdata(pdev, gpio_data);
    return 0;
}

static int timed_gpio_remove(struct platform_device *pdev)
{
    struct timed_gpio_platform_data *pdata = pdev->dev.platform_data;
    struct timed_gpio_data *gpio_data = platform_get_drvdata(pdev);
    int i;

    if (!pdata || !gpio_data)
        return 0;

    for (i = 0; i < pdata->num_gpios; i++)
        timed_output_dev_unregister(&gpio_data[i].dev);

    kfree(gpio_data);
    return 0;
}

static struct platform_driver timed_gpio_driver = {
    .probe  = timed_gpio_probe,
    .remove = timed_gpio_remove,
    .driver = {
        .name  = TIMED_GPIO_NAME,
        .owner = THIS_MODULE,
    },
};

static int __init timed_gpio_init(void)
{
    return platform_driver_register(&timed_gpio_driver);
}

static void __exit timed_gpio_exit(void)
{
    platform_driver_unregister(&timed_gpio_driver);
}

module_init(timed_gpio_init);
module_exit(timed_gpio_exit);

MODULE_AUTHOR("Mike Lockwood <lockwood@android.com>");
MODULE_DESCRIPTION("timed gpio driver");
MODULE_LICENSE("GPL");
