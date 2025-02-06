#ifndef __CAMERA_DEVICE_HPP__
#define __CAMERA_DEVICE_HPP__


#include <string>
#include "../incEn/HCNetSDK.h"

struct CameraDevice {
    const int MAX_USERNAME_LENGTH = 32;
    const int MAX_PASSWORD_LENGTH = 32;
    
    bool isSdkInitialized = false;
    int loggedUserId = -1;
    int lastError = 0;
    int lRealHandle = -1;


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
        NET_DVR_USER_LOGIN_INFO loginInfo;
        NET_DVR_DEVICEINFO_V40 deviceInfoV40;
        loginInfo.bUseAsynLogin = 0;
        strncpy(
            loginInfo.sUserName, username.c_str(),
             MAX_USERNAME_LENGTH);
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


    ~CameraDevice() {
        if (isSdkInitialized) {

            NET_DVR_Cleanup();
        }
    }

};

#endif // __CAMERA_DEVICE_HPP__
