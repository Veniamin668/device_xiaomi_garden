/*
 * Copyright (C) 2026 AERA Recovery Project contributors
 * SPDX-License-Identifier: Apache-2.0
 *
 * MediaTek MT6765/MT6762 ALSA PCM audio bridge for Redmi 9A / 9C (garden/angelica).
 * Untrusted plugins never receive direct access to ALSA or root capabilities.
 * Browser, media, and game UIDs connect via abstract socket "aera-browser-audio-v1"
 * and stream 16-bit 48kHz stereo PCM.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <sound/asound.h>
#include <cutils/log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "aera-audio-bridge"

namespace {

constexpr uid_t kBrowserUid = 99090;
constexpr uid_t kMediaUid = 99092;
constexpr uid_t kDoomUid = 99094;
constexpr uint32_t kMagic = 0x41525041U; // "APRA"
constexpr uint32_t kRate = 48000U;
constexpr uint32_t kChannels = 2U;
constexpr uint32_t kBits = 16U;
constexpr char kSocketName[] = "aera-browser-audio-v1";

constexpr const char* kPcmDevices[] = {
    "/dev/snd/pcmC0D0p",   // Primary MTK DL1 Playback
    "/dev/snd/pcmC0D1p",
    "/dev/snd/pcmC0D15p",
};

constexpr const char* kMixerControls[] = {
    "Audio_Amp_L_Switch",
    "Audio_Amp_R_Switch",
    "Speaker_Amp_Switch",
    "Ext_Speaker_Amp_Switch",
    "Audio_Speaker_class_Switch",
};

volatile sig_atomic_t g_stop = 0;

void SignalHandler(int) {
    g_stop = 1;
}

int ReadAll(int fd, void* output, size_t size) {
    uint8_t* cursor = static_cast<uint8_t*>(output);
    while (size > 0 && !g_stop) {
        const ssize_t count = read(fd, cursor, size);
        if (count > 0) {
            cursor += count;
            size -= static_cast<size_t>(count);
            continue;
        }
        if (count < 0 && errno == EINTR) continue;
        return 0;
    }
    return size == 0 ? 1 : 0;
}

void SetMixerControl(const char* control_name, int value) {
    const int ctl_fd = open("/dev/snd/controlC0", O_RDWR | O_CLOEXEC);
    if (ctl_fd < 0) return;

    struct snd_ctl_elem_list list {};
    if (ioctl(ctl_fd, SNDRV_CTL_IOCTL_ELEM_LIST, &list) < 0 || list.count == 0) {
        close(ctl_fd);
        return;
    }

    struct snd_ctl_elem_id* ids = static_cast<struct snd_ctl_elem_id*>(
        calloc(list.count, sizeof(struct snd_ctl_elem_id)));
    if (!ids) {
        close(ctl_fd);
        return;
    }

    list.space = list.count;
    list.pids = ids;
    if (ioctl(ctl_fd, SNDRV_CTL_IOCTL_ELEM_LIST, &list) >= 0) {
        for (unsigned int i = 0; i < list.used; ++i) {
            if (strncmp(reinterpret_cast<const char*>(ids[i].name), control_name, sizeof(ids[i].name)) == 0) {
                struct snd_ctl_elem_value val {};
                val.id = ids[i];
                val.value.integer.value[0] = value;
                val.value.integer.value[1] = value;
                ioctl(ctl_fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &val);
                break;
            }
        }
    }

    free(ids);
    close(ctl_fd);
}

void EnableSpeakerHardware() {
    for (const char* name : kMixerControls) {
        SetMixerControl(name, 1);
    }
}

int OpenPcmDevice() {
    for (const char* dev_path : kPcmDevices) {
        int fd = open(dev_path, O_RDWR | O_CLOEXEC);
        if (fd < 0) continue;

        struct snd_pcm_hw_params params {};
        // Initialize default hw params
        struct snd_mask* mask = reinterpret_cast<struct snd_mask*>(
            &params.masks[SNDRV_PCM_HW_PARAM_ACCESS - SNDRV_PCM_HW_PARAM_FIRST_MASK]);
        mask->bits[0] = (1U << SNDRV_PCM_ACCESS_RW_INTERLEAVED);

        mask = reinterpret_cast<struct snd_mask*>(
            &params.masks[SNDRV_PCM_HW_PARAM_FORMAT - SNDRV_PCM_HW_PARAM_FIRST_MASK]);
        mask->bits[0] = (1U << SNDRV_PCM_FORMAT_S16_LE);

        struct snd_interval* interval = &params.intervals[SNDRV_PCM_HW_PARAM_CHANNELS - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
        interval->min = kChannels;
        interval->max = kChannels;

        interval = &params.intervals[SNDRV_PCM_HW_PARAM_RATE - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
        interval->min = kRate;
        interval->max = kRate;

        interval = &params.intervals[SNDRV_PCM_HW_PARAM_PERIOD_SIZE - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
        interval->min = 1024;
        interval->max = 1024;

        interval = &params.intervals[SNDRV_PCM_HW_PARAM_PERIODS - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
        interval->min = 4;
        interval->max = 4;

        if (ioctl(fd, SNDRV_CTL_IOCTL_PCM_PREFER_SUBDEVICE, 0) < 0) {
            // non-fatal
        }

        if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &params) >= 0) {
            struct snd_pcm_sw_params sw_params {};
            sw_params.tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
            sw_params.period_step = 1;
            sw_params.avail_min = 1024;
            sw_params.start_threshold = 1024;
            sw_params.stop_threshold = 4096;

            if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &sw_params) >= 0 &&
                ioctl(fd, SNDRV_PCM_IOCTL_PREPARE) >= 0) {
                return fd;
            }
        }

        close(fd);
    }
    return -1;
}

void ProcessClient(int client_fd) {
    struct ucred cred {};
    socklen_t cred_len = sizeof(cred);
    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &cred, &cred_len) != 0 ||
        cred_len != sizeof(cred)) {
        return;
    }

    if (cred.uid != kBrowserUid && cred.uid != kMediaUid &&
        cred.uid != kDoomUid && cred.uid != 0) {
        return;
    }

    uint32_t hello[4] = {0};
    if (!ReadAll(client_fd, hello, sizeof(hello)) ||
        hello[0] != kMagic || hello[1] != kRate ||
        hello[2] != kChannels || hello[3] != kBits) {
        return;
    }

    EnableSpeakerHardware();
    int pcm_fd = OpenPcmDevice();

    uint8_t buffer[4096];
    while (!g_stop) {
        const ssize_t count = read(client_fd, buffer, sizeof(buffer));
        if (count > 0) {
            if (pcm_fd >= 0) {
                struct snd_xferi xferi {};
                xferi.buf = buffer;
                xferi.frames = static_cast<snd_pcm_uframes_t>(count / (kChannels * (kBits / 8)));
                if (ioctl(pcm_fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &xferi) < 0) {
                    if (errno == EPIPE || errno == ESTRPIPE) {
                        ioctl(pcm_fd, SNDRV_PCM_IOCTL_PREPARE);
                        ioctl(pcm_fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &xferi);
                    }
                }
            }
            continue;
        }
        if (count < 0 && errno == EINTR) continue;
        break;
    }

    if (pcm_fd >= 0) {
        ioctl(pcm_fd, SNDRV_PCM_IOCTL_DRAIN);
        close(pcm_fd);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2 || strcmp(argv[1], "--browser-audio") != 0 || getuid() != 0) {
        fprintf(stderr, "Usage (root recovery only): aera-audio-bridge --browser-audio\n");
        return 64;
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, SignalHandler);
    signal(SIGINT, SignalHandler);
    prctl(PR_SET_PDEATHSIG, SIGTERM, 0, 0, 0);

    const int server = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (server < 0) return 70;

    struct sockaddr_un address {};
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path + 1, kSocketName, sizeof(kSocketName) - 1);
    const socklen_t address_size = static_cast<socklen_t>(
        offsetof(struct sockaddr_un, sun_path) + 1 + sizeof(kSocketName) - 1);

    if (bind(server, reinterpret_cast<const struct sockaddr*>(&address), address_size) != 0 ||
        listen(server, 2) != 0) {
        close(server);
        return 70;
    }

    fprintf(stderr, "AERA audio: MTK MT6765 audio bridge ready\n");

    while (!g_stop) {
        const int client = accept4(server, nullptr, nullptr, SOCK_CLOEXEC);
        if (client < 0) {
            if (errno == EINTR) continue;
            break;
        }
        ProcessClient(client);
        close(client);
    }

    close(server);
    return 0;
}
