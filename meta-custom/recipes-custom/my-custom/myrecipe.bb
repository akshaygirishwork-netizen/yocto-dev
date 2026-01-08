SUMMARY = "My recipe"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://test.sh"

S="${WORKDIR}"

do_compile() {

${CC} ${CFLAGS} ${LDFLAGS} -o myexecutable main.c 

}

do_install() {
    install -d ${D}${bindir}
    install -d ${D}/opt/pd/bin
    install -m 0755 myexecutable ${D}${bindir}/myexecutable
    install -m 0644 test.sh ${D}/opt/pd/bin/test.sh
}

FILES:${PN} += " /opt/pd/bin/test.sh"