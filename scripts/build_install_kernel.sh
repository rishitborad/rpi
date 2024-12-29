#!/bin/bash

# WIKI: https://www.raspberrypi.com/documentation/computers/linux_kernel.html#building

LINUX_SRC=~/linux

# Define the kernel variable
ARCH=arm
KERNEL="kernel7l"
DATE_TIME=$(date +"%Y-%m-%d_%H-%M")
CUSTOM_KERNEL="-v7l-rb-custom"

# Navigate to the linux directory
pushd ${LINUX_SRC} || { echo "Directory 'linux' not found"; exit 1; }

echo "Preparing default kernel config"
make bcm2711_defconfig

# Find .config file and set CONFIG_LOCALVERSION to ${KERNEL}-rishit-custom-${DATE_TIME}
if [[ -f ".config" ]]; then
  echo "Modifying .config file..."
  sed -i "s/^CONFIG_LOCALVERSION=.*/CONFIG_LOCALVERSION=\"${CUSTOM_KERNEL}-${DATE_TIME}\"/" .config

  # Check if sed was successful
  if [[ $? -ne 0 ]]; then
    echo "Error: Failed to modify the .config file."
    exit 1
  fi

  # Check if the modification was successful
  UPDATED_CONFIG=$(grep CONFIG_LOCALVERSION= .config)
  echo "Updated CONFIG_LOCALVERSION: ${UPDATED_CONFIG}"
else
  echo ".config file not found"
  exit 1
fi

# Build the kernel and modules
echo "Configuring and building the kernel..."
make -j6 zImage modules dtbs

# Install the modules
echo "Installing modules..."
sudo make -j6 modules_install

# Backup the current kernel image
# echo "Backing up the current kernel image..."
# sudo cp /boot/$KERNEL.img /boot/$KERNEL-backup.img

# Copy the new kernel image and device tree blobs
echo "Copying new kernel image and device tree blobs..."
sudo cp ~/linux/arch/arm/boot/zImage /boot/$KERNEL.img
sudo cp ~/linux/arch/arm/boot/dts/*.dtb /boot/
sudo cp ~/linux/arch/arm/boot/dts/overlays/*.dtb* /boot/overlays/
sudo cp ~/linux/arch/arm/boot/dts/overlays/README /boot/overlays/

# Reboot the system
echo "Rebooting the system..."
sudo reboot