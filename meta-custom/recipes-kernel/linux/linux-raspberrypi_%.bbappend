FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://mychardev-overlay.dts"

# Tell kernel to build the overlay
KERNEL_DEVICETREE:append = " overlays/mychardev-overlay.dtbo"
# Safely copy DTS into kernel tree before compile
do_configure:append() {
    install -m 0644 ${WORKDIR}/mychardev-overlay.dts \
        ${S}/arch/${ARCH}/boot/dts/overlays/
}