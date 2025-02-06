#ifndef __CAMERA_DEVICE_HPP__
#define __CAMERA_DEVICE_HPP__


#include <string>
#include "../incEn/HCNetSDK.h"

struct CameraDevice {
    
    std::string ip;
    int port;
    std::string username;
    std::string password;
    CameraDevice( 
        const std::string& _ip,
         int _port,
          const std::string& _username, const std::string& _password)
        : ip(_ip)
        , port(_port)
        , username(_username)
        , password(_password)
    {
        NET_DVR_Init();
    }
   
};

#endif // __CAMERA_DEVICE_HPP__
