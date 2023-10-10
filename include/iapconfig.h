#ifndef __IAPCONFIG_H
#define __IAPCONFIG_H

/* STM32F4xx specific HAL configuration options. */
#if __has_include("iapconfig_custom.h")
#include "iapconfig_custom.h"
#else
#include "iapconfig_default.h"
#endif

#endif /* IAPCONFIG */