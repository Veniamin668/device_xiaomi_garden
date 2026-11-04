
#
# This file is part of the AERA Recovery Project
# Original device configuration derived from the garden OrangeFox tree.
#
# Copyright (C) 2022 The OrangeFox Recovery Project
# Copyright (C) 2026 AERA Recovery Project device adaptation
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Please maintain this file if you use it or any part of it.
#

FDEVICE="garden"

aera_get_target_device() {
    local chkdev

    chkdev=$(echo "$BASH_SOURCE" | grep -w "$FDEVICE")

    if [ -n "$chkdev" ]; then
        AERA_BUILD_DEVICE="$FDEVICE"
    else
        chkdev=$(set | grep BASH_ARGV | grep -w "$FDEVICE")
        [ -n "$chkdev" ] && AERA_BUILD_DEVICE="$FDEVICE"
    fi
}

if [ -z "$1" ] && [ -z "$AERA_BUILD_DEVICE" ]; then
    aera_get_target_device
fi

if [ "$1" = "$FDEVICE" ] || [ "$AERA_BUILD_DEVICE" = "$FDEVICE" ]; then

    export LC_ALL="C"
    export ALLOW_MISSING_DEPENDENCIES=true

    # AERA build identity
    export AERA_BUILD_DEVICE="$FDEVICE"
    export AERA_BUILD_STATUS="Unofficial"
    export AERA_BUILD_TYPE="Beta"
    export AERA_PRODUCT_PREFIX="AERA"
    export AERA_VANILLA_BUILD=1

    # Device codenames
    export TARGET_DEVICE_ALT="dandelion,angelica,angelican,angelicain,cattail"
    export AERA_TARGET_DEVICES="$TARGET_DEVICE_ALT"

    # Recovery partition paths
    export AERA_RECOVERY_INSTALL_PARTITION="/dev/block/platform/bootdevice/by-name/recovery"
    export AERA_RECOVERY_SYSTEM_PARTITION="/dev/block/mapper/system"
    export AERA_RECOVERY_VENDOR_PARTITION="/dev/block/mapper/vendor"
    export AERA_RECOVERY_BOOT_PARTITION="/dev/block/platform/bootdevice/by-name/boot"

    # AERA tools
    export AERA_USE_TAR_BINARY=1
    export AERA_USE_SED_BINARY=1
    export AERA_USE_LZ4_BINARY=1
    export AERA_USE_ZSTD_BINARY=1
    export AERA_USE_DATE_BINARY=1
    export AERA_USE_GREP_BINARY=1
    export AERA_USE_BUSYBOX_BINARY=1
    export AERA_USE_XZ_UTILS=1
    export AERA_USE_UPDATED_MAGISKBOOT=1
    export AERA_USE_FSCK_EROFS_BINARY=1
    export AERA_USE_PATCHELF_BINARY=1

    # Magisk image patching
    export AERA_USE_MAGISKBOOT=1
    export AERA_USE_MAGISKBOOT_FOR_ALL_PATCHES=1
    export AERA_DELETE_MAGISK_ADDON=1

    # AROMA
    export AERA_DELETE_AROMAFM=1

    # Settings storage
    export AERA_SETTINGS_ROOT_DIRECTORY="/data/recovery"
    export AERA_MISCELLANEOUS_ROOT_DIRECTORY="/sdcard"

    # Dynamic partitions
    export AERA_USE_DMSETUP=1

    # OrangeFox-compatible behavior supported by AERA
    export AERA_DONT_PATCH_ENCRYPTED_DEVICE=1
    export AERA_NO_TREBLE_COMPATIBILITY_CHECK=1
    export AERA_NO_MIUI_PATCH_WARNING=1

    # Backup
    export AERA_QUICK_BACKUP_LIST="/boot;/data;"

    # AVB
    export AERA_PATCH_AVB20=1

    # OTA
    export AERA_KEEP_DM_VERITY=1
    export AERA_SUPPORT_ALL_BLOCK_OTA_UPDATES=1
    export AERA_FIX_OTA_UPDATE_MANUAL_FLASH_ERROR=1
    export AERA_DISABLE_MIUI_OTA_BY_DEFAULT=1

    # UI
    export AERA_USE_GREEN_LED=0
    export AERA_HIDE_NOTCH=1

    # AERA starts its own versioning.
    # Do not carry over the OrangeFox maintainer patch suffix.
    unset AERA_MAINTAINER_PATCH_VERSION

fi
