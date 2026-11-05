#
# AERA Recovery Project
# Device: Xiaomi Redmi 9A
# Codename: garden
# Platform: MediaTek MT6765
#

# Base Android product configuration
$(call inherit-product, $(SRC_TARGET_DIR)/product/base.mk)

# TWRP / AERA common recovery configuration
$(call inherit-product, vendor/twrp/config/common.mk)

# Device-specific configuration
$(call inherit-product, device/xiaomi/garden/device.mk)

#
# Product identity
#

PRODUCT_DEVICE := garden
PRODUCT_NAME := twrp_garden
PRODUCT_BRAND := Xiaomi
PRODUCT_MODEL := Redmi 9A
PRODUCT_MANUFACTURER := Xiaomi

#
# AERA maintainer information
#

AERA_MAINTAINER := ktoya?

#
# AERA display configuration
#

AERA_SCREEN_H := 2400
AERA_STATUS_H := 80
AERA_STATUS_INDENT_LEFT := 48
AERA_STATUS_INDENT_RIGHT := 48
AERA_CLOCK_POS := 1
AERA_ALLOW_DISABLE_NAVBAR := 0

#
# AERA device UI
#

AERA_HIDE_NOTCH := 1
AERA_USE_GREEN_LED := 0

#
# Recovery behavior
#

AERA_NO_TREBLE_COMPATIBILITY_CHECK := 1
AERA_QUICK_BACKUP_LIST := "/boot;/data;"

#
# Dynamic partitions
#

AERA_ENABLE_ALL_PARTITION_TOOLS := 1

#
# Filesystem
#

AERA_USE_LZ4_COMPRESSION := 1

#
# Kernel
#

AERA_FORCE_PREBUILT_KERNEL := 1
