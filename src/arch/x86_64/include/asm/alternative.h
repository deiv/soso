
#pragma once

#ifdef CONFIG_SMP

#define LOCK_PREFIX LOCK_PREFIX_HERE "\tlock;"

#else /* ! CONFIG_SMP */

#define LOCK_PREFIX ""

#endif
