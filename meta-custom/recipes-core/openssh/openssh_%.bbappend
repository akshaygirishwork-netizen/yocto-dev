FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://sshd@.service"

# Tell Yocto to enable this service at boot
SYSTEMD_SERVICE:${PN} = "sshd@.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

