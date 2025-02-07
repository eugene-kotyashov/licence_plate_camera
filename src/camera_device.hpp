#ifndef __CAMERA_DEVICE_HPP__
#define __CAMERA_DEVICE_HPP__


#include <string>
#include "../incEn/HCNetSDK.h"
#include "alarm_callback.hpp"

struct CameraDevice {
    const int MAX_USERNAME_LENGTH = 32;
    const int MAX_PASSWORD_LENGTH = 32;
    
    bool isSdkInitialized = false;
    int loggedUserId = -1;
    int lastError = 0;
    LONG lHandle = -1;


    CameraDevice()

    {
        isSdkInitialized = NET_DVR_Init();
        if (isSdkInitialized) {
            NET_DVR_SetLogToFile(3, "sdk_log", true);
        }
    }

    void connect(
        const std::string& ip,
        int port,
        const std::string& username,
        const std::string& password
    ) {
        NET_DVR_USER_LOGIN_INFO loginInfo = {0};
        NET_DVR_DEVICEINFO_V40 deviceInfoV40 = {0};
        loginInfo.bUseAsynLogin = 0;
        strncpy(
            loginInfo.sUserName, username.c_str(),
             MAX_USERNAME_LENGTH);
             
        strcpy(loginInfo.sDeviceAddress, ip.c_str());
        strncpy(
            loginInfo.sPassword, password.c_str(),
             MAX_PASSWORD_LENGTH);

        loginInfo.wPort = port;
        loggedUserId = NET_DVR_Login_V40(
             &loginInfo, &deviceInfoV40);


        if (loggedUserId < 0) {
            lastError = NET_DVR_GetLastError();
        }
    }

    void disconnect() {
        if (!isSdkInitialized) return;
        if (loggedUserId < 0) return;
        if (!NET_DVR_Logout(loggedUserId)) {
            lastError = NET_DVR_GetLastError();
            return;
        }
        loggedUserId = -1;
    }

    void enableArming()
    {
        if (!isSdkInitialized)
            return;
        if (loggedUserId < 0)
            return;
        NET_DVR_SETUPALARM_PARAM struSetupParam = {0};
        struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);
        // Alarm information type to upload:
        // 0-History Alarm (NET_DVR_PLATE_RESULT), 1-Real-Time Alarm
        // (NET_ITS_PLATE_RESULT)
        struSetupParam.byAlarmInfoType = 1;
        // Arming Level: Level-2 arming (for traffic device)
        struSetupParam.byLevel = 1;
        lHandle = NET_DVR_SetupAlarmChan_V41(loggedUserId, &struSetupParam);
        if (lHandle < 0)
        {
            lastError = NET_DVR_GetLastError();
            return;
        }

        if (!NET_DVR_SetDVRMessageCallBack_V50(
            0, GetLicencePlatePicsAndText, nullptr
        )) {
            lastError = NET_DVR_GetLastError();
        }
    }


    void disableArming() {
        if (!isSdkInitialized) return;
        if (loggedUserId < 0) return;
        if (!NET_DVR_CloseAlarmChan_V30(lHandle))
        {
            lastError = NET_DVR_GetLastError();
        }
        NET_DVR_SetDVRMessageCallBack_V50(
            0, nullptr, nullptr
        );

    }


    ~CameraDevice() {
        if (isSdkInitialized) {

            NET_DVR_Cleanup();
        }
    }

};

#endif // __CAMERA_DEVICE_HPP__
