
#ifndef __SETUP_H__
#define __SETUP_H__

#ifdef __cplusplus
 extern "C" {
#endif
#include "flash_if.h"

void user_setup(); 
void user_loop(); 
typedef  void (*pFunction)(void);
extern char _estack;

const uint32_t*  __attribute__((used, section (".iap_const"))) user_setup_address = (uint32_t*)user_setup;
const uint32_t*  __attribute__((used, section (".iap_const"))) user_loop_address = (uint32_t*)user_loop;

void  __IAP_FUNC setup(){
 
    pFunction jumpFunction = (pFunction) user_setup_address;
    __set_MSP((uint32_t)&_estack);
    jumpFunction();

    loop();
}

void  __IAP_FUNC loop(){
    pFunction jumpFunction = (pFunction) user_loop_address;
    jumpFunction();
}

#ifdef __cplusplus
}
#endif

#endif /* __SETUP_H__ */