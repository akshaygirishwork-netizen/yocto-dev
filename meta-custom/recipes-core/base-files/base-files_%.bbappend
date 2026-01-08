
do_install:append() {
    echo "/dev/mmcblk0p1  /boot  vfat  defaults  0  2" >> ${D}${sysconfdir}/fstab

    echo "raspberrypi3 1.0" > ${D}${sysconfdir}/hwrevision

    echo "akshay1" > ${D}${sysconfdir}/akshay
}