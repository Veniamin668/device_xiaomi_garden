
#
# AERA Recovery Project device configuration
# Device: Xiaomi Redmi 9A
# Codename: garden
# Platform: MediaTek MT6765
#
# Original OrangeFox maintainer information preserved.
#

# Maintainer
AERA_MAINTAINER := ktoya?

# Display
AERA_SCREEN_H := 2400
AERA_STATUS_H := 80
AERA_STATUS_INDENT_LEFT := 48
AERA_STATUS_INDENT_RIGHT := 48
AERA_CLOCK_POS := 1
AERA_ALLOW_DISABLE_NAVBAR := 0

# Device UI
AERA_HIDE_NOTCH := 1
AERA_USE_GREEN_LED := 0

# Recovery behavior
AERA_NO_TREBLE_COMPATIBILITY_CHECK := 1
AERA_QUICK_BACKUP_LIST := "/boot;/data;"

# Dynamic partitions
AERA_ENABLE_ALL_PARTITION_TOOLS := 1

# Device-specific filesystem behavior
AERA_USE_LZ4_COMPRESSION := 1

# Keep the device's existing recovery kernel.
AERA_FORCE_PREBUILT_KERNEL := 1
