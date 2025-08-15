#!/bin/bash

make clean
make 
 
sudo insmod ioctl_device_driver.ko 

sudo mknod /dev/ioctl_device_driver c $(grep ioctl_device_driver /proc/devices | awk '{print $1}') 0
sudo chmod 666 /dev/ioctl_device_driver

gcc -o user user.c
sudo ./user 5

sudo rmmod ioctl_device_driver
sudo rm /dev/ioctl_device_driver


