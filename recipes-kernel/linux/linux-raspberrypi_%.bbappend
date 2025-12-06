FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Apply your DT patch
SRC_URI += "file://0001-add-mychardev-node.patch"