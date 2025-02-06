#ifndef __CAMERA_DEVICE_HPP__
#define __CAMERA_DEVICE_HPP__


#include <string>
#include "../incEn/HCNetSDK.h"

struct CameraDevice {
    
    
    bool isSdkInitialized = false;
    CameraDevice()
    {
        isSdkInitialized = NET_DVR_Init();
        if (isSdkInitialized) {
            NET_DVR_SetLogToFile(3, "sdk_log", true);
        }


    }



    ~CameraDevice() {
        if (isSdkInitialized) {
            NET_DVR_Cleanup();
        }
    }

};

#endif // __CAMERA_DEVICE_HPP__
