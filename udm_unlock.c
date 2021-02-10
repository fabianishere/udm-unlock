/*
 * udm_unlock
 *
 * Copyright (C) 2021 Fabian Mastenbroek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define MODULE_NAME "udm_unlock"
#define pr_fmt(fmt) MODULE_NAME ": " fmt

#include <linux/module.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/genhd.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Fabian Mastenbroek <mail.fabianm@gmail.com>");
MODULE_DESCRIPTION("Unlock write-protected disks on the UDM Pro");
MODULE_VERSION("1.0");

static ssize_t udm_unlock_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct hd_struct *part = dev_to_part(dev);
	struct gendisk *disk = part_to_disk(part);
	int flag = part->partno ? part->ubnt_readonly : disk->ubnt_readonly;

	return sprintf(buf, "%d\n", flag ? 0 : 1);
}

static ssize_t udm_unlock_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct hd_struct *part = dev_to_part(dev);
	struct gendisk *disk = part_to_disk(part);
	int flag;

	/* We assume buf is null-terminated, since this function does not
	 * accept a count */
	if (kstrtoint(buf, 0, &flag)) {
		return -EINVAL;
	}

	if (flag != 0 && flag != 1) {
		return -EINVAL;
	}

	if (part->partno) {
		ubnt_set_part_ro(disk, part->partno, !flag);
	} else {
		ubnt_set_disk_ro(disk, !flag);
	}

	return count;
}

static DEVICE_ATTR(udm_unlock, S_IRUGO | S_IWUSR, udm_unlock_show, udm_unlock_store);

static void udm_unlock_register_gendisk(struct gendisk *disk, struct hd_struct *part, void *priv)
{
	int err;
	struct device *dev;

	if (part->partno) {
		pr_info("Adding entry to partition: %s%d\n", disk->disk_name, part->partno);
		dev = part_to_dev(part);
	} else {
		pr_info("Adding entry to disk: %s\n", disk->disk_name);
		dev = disk_to_dev(disk);
	}

	err = device_create_file(dev, &dev_attr_udm_unlock);
	if (err < 0) {
		pr_warn("Failed to register sysfs object: %d\n", err);
	}

}

static void udm_unlock_unregister_gendisk(struct gendisk *disk, struct hd_struct *part, void *priv)
{
	struct device *dev = part_to_dev(part);

	device_remove_file(dev, &dev_attr_udm_unlock);
}

static int __init
udm_unlock_init(void)
{
	/* Iterate over all disks */
	gendisk_callback_for_each(udm_unlock_register_gendisk, NULL);

	/* Watch for new disks */
	gendisk_event_hook_reg(udm_unlock_register_gendisk, NULL, DISK_EVENT_ADD);
	return 0;
}

module_init(udm_unlock_init)

static void __exit
udm_unlock_exit(void)
{
	pr_info("Stopping...\n");

	/* Remove event hook */
	gendisk_event_hook_unreg(udm_unlock_register_gendisk, DISK_EVENT_ADD);

	/* Remove all sysfs entries */
	gendisk_callback_for_each(udm_unlock_unregister_gendisk, NULL);
}
module_exit(udm_unlock_exit);
