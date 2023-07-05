
#pragma once

struct legacy_pic {
    void (*init)(int auto_eoi);
    int (*probe)(void);
};
