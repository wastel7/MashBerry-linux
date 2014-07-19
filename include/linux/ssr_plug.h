#ifndef __LINUX_SSR_PLUG_H
#define __LINUX_SSR_PLUG_H

#define SSR_PLUG_DRIVER_NAME "ssr_plug"
#define SSR_INT_DIV 2

struct ssr_plug_config_entry {
    int gpio;
    int power;    // 0-100%, -1 is off
    int gpioval;
};

struct ssr_plug_config {
    int plugCnt;
    struct ssr_plug_config_entry *ssr_plug_config;
    int initialized;
    spinlock_t ssr_lock;
};

#endif
