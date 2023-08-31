
#pragma once

struct legacy_pic {
    void (*init)(int auto_eoi);
    int (*probe)(void);
    void (*mask)(unsigned int irq);
    void (*eoi)(unsigned int irq);
};
