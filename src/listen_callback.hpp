#ifndef LISTEN_CALLBACK_HPP
#define LISTEN_CALLBACK_HPP

#include "HCNetSDK.h"
#include <iostream>

void CALLBACK ListenCallback(
    LONG lCommand,
    NET_DVR_ALARMER *pAlarmer,
    char *pAlarmInfo,
    DWORD dwBufLen,
    void *pUser)
{
    std::cout << "listen callback is called!" << std::endl;   
}

#endif