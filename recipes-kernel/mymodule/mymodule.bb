SUMMARY = "Simple kernel module for Raspberry Pi"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://mymodule.c;beginline=1;endline=30;md5=e7a6f6fc5c9809b81a94ecb1213b1623"

inherit module

SRC_URI = "file://mymodule.c \
           file://Makefile"

S = "${WORKDIR}"

# Correct kernel headers will be used automatically