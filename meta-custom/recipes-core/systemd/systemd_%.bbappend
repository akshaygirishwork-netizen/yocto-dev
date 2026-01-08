FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://01-eth0.network"

do_install:append() {
    install -d ${D}/lib/systemd/network
    install -m 0644 ${WORKDIR}/01-eth0.network ${D}/etc/systemd/network/
}
