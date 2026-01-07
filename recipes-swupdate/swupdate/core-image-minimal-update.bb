DESCRIPTION = "SWUpdate image for RPi"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

COMPATIBLE_MACHINE = "raspberrypi3-64"

inherit image swupdate

SRC_URI = "file://sw-description"

SWUPDATE_IMAGES = "core-image-minimal"

IMAGE_DEPENDS = "core-image-minimal"

SWUPDATE_IMAGES_FSTYPES[core-image-minimal] = ".ext4"

IMAGE_INSTALL:append = " busybox libgcc"