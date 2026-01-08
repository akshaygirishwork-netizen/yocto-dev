WKS_FILE = "core-image-minimal.wks"

IMAGE_FEATURES += " ssh-server-openssh"
IMAGE_FSTYPES += " wic ext4"

IMAGE_INSTALL:append = " mymodule \
                         mycustom \
                         myrecipe \
                         busybox \
                         u-boot-fw-utils \
                         swupdate \
                         libgcc \ 
                         e2fsprogs \
                         swupdate-tools \
                         util-linux-lsblk"

