DESCRIPTION = "SWUpdate image for RPi"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# CRITICAL: Add machine compatibility
COMPATIBLE_MACHINE = "raspberrypi3-64"

inherit swupdate

SRC_URI = "file://sw-description"

SWUPDATE_IMAGES = "core-image-minimal-raspberrypi3-64"

# SWUPDATE_IMAGES[core-image-minimal-${MACHINE}] = ".ext4"

# Optional but recommended: Specify the filesystem type
# do_swuimage[depends] += "core-image-minimal:do_image_complete"

SWUPDATE_IMAGES_FSTYPES[core-image-minimal-raspberrypi3-64] = ".ext4"
