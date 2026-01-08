# Enable FIT image creation for U-Boot (manual signing)
FILESEXTRAPATHS:prepend := "${THISDIR}/u-boot:"

SRC_URI += "file://fit-sign.cfg"
