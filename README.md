# udm-unlock
Unlock write-protected disks on the UDM Pro.

## Purpose
This loadable kernel module enables users of the UDM Pro to mount the
write-protected disks as read-write. Ubiquiti uses a custom kernel module to
mark the disks as write-protected to prevent users from modifying the disks.
This kernel module overrides this kernel setting and marks the disks as 
read-write.

Note that it is not possible to remount the root filesystem as read-write with
this kernel module as the root partition is a read-only SquashFS image. If
you really must make changes to the root partition, you must extract the 
SquashFS image, apply the changes, rebuild the SquahsFS image and overwrite the
partition.

## Getting the source
Download the source code by running the following code in your command prompt:
```sh
$ git clone https://github.com/fabianishere/udm-unlock.git
```
or simply [grab](https://github.com/fabianishere/udm-unlock/archive/master.zip) a copy of the source code as a Zip file.

## Building
Before you start, make sure you have installed the following packages for
the build environment:

```bash
sudo apt install gcc-aarch64-linux-gnu
```

Now nagivate to the root directory of this repository and export the path to 
the Linux kernel sources against you wish to build:

```bash
export KERNEL=/root/linux
```
Then, build the module using `make`:
```bash
make
```
This will build `udm_unlock.ko` which can be loaded into the Linux kernel.

## Usage
**Warning**
You may render your device unusable by messing with the partitions of your
device. Make sure you know what you are doing and ensure you have a backup!

To start, make sure you have loaded the kernel module:

```bash
insmod udm_unlock.ko
```

Then, unlock one of the disks as follows:
```bash
echo 1 > /sys/class/block/sdb1/udm_unlock
```

To lock the disk again, you may write zero to the sysfs file:
```bash
echo 0 > /sys/class/block/sdb1/udm_unlock
```

## License
The code is released under the GPLv2 license. See [COPYING.txt](/COPYING.txt).
