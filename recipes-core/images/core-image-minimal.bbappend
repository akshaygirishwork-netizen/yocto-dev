WKS_FILE = "core-image-minimal.wks"

IMAGE_FEATURES += " ssh-server-openssh"
IMAGE_FSTYPES += " wic ext4"

IMAGE_INSTALL:append = " openssh openssh-sshd dtc"

IMAGE_INSTALL:append = " mymodule mycustom myrecipe"
