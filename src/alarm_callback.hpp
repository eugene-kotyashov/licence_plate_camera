#ifndef ALARM_CALLBACK_HPP
#define ALARM_CALLBACK_HPP
#include "HCNetSDK.h"
#include "image_list_table.hpp"
#include <FL/Fl_JPEG_Image.H>
#include <cstdio>
#include <string>
#include <sstream>

void addAlarmResultView(
    ImageListTable *table,
    const unsigned char *vehicleImageBuf,
    const unsigned char *plateImageBuf,
    const std::string &licensePlate,
    const std::string &firstPicTimeStr
)

{
    Fl_Image *veh = new Fl_PNG_Image("vehicle.png");   
    Fl_Image *plate = new Fl_PNG_Image("plate.png");

    if (vehicleImageBuf != nullptr)
    {
        delete veh;
        veh = new Fl_JPEG_Image(nullptr, vehicleImageBuf);
        if (veh->fail())
        {
            delete veh;
            printf("Failed to load vehicle image\n");
            veh = new Fl_PNG_Image("vehicle.png"); 
        }
       
    } else {
        printf("vehicle image buffer is nullptr\n");
    }

    if (plateImageBuf != nullptr) {
        delete plate;
        plate = new Fl_JPEG_Image(nullptr, plateImageBuf);
        if (plate->fail())
        {
            delete plate;
            printf("Failed to load plate image\n");
            plate = new Fl_PNG_Image("plate.png");
        }
    } else {
        printf("plate image buffer is nullptr\n");
    }
   
    ListItem *item = new ListItem(
        *plate, licensePlate, firstPicTimeStr, table->getItemCount(),
        "US", "Forward");
    Fl::lock();
    printf("table has  %d items\n", table->getItemCount());
    table->addItem(*item);
    Fl::unlock();
    Fl::awake();

}



void CALLBACK GetLicencePlatePicsAndText(
    LONG lCommand,
    NET_DVR_ALARMER *pAlarmer,
    char *pAlarmInfo,
    DWORD dwBufLen,
    void *pUser)
{   
    printf("alarm callback called\n");
    ImageListTable* table = (ImageListTable* )pUser;
    switch (lCommand)
    {
    case COMM_ALARM:
    {
        NET_DVR_ALARMINFO struAlarmInfo;
        printf("Alarm\n");
        memcpy(&struAlarmInfo, pAlarmInfo, sizeof(NET_DVR_ALARMINFO));
        break;
    }
    case COMM_ALARM_V30:
    {
        NET_DVR_ALARMINFO_V30 struAlarmInfoV30;
        printf("Alarm V30\n");
        break;
    }

    case COMM_ALARM_V40:
    {
        NET_DVR_ALARMINFO_V40 struAlarmInfoV40;
        memcpy(&struAlarmInfoV40, pAlarmInfo, sizeof(NET_DVR_ALARMINFO_V40));
        printf("Alarm V40\n");
        if ((struAlarmInfoV40.struAlarmFixedHeader.dwAlarmType == 0) ||
         (struAlarmInfoV40.struAlarmFixedHeader.dwAlarmType == 23)) {
            printf( "Alarm io alarm\n" );
        }
        
        break;
    }
    case COMM_VEHICLE_CONTROL_ALARM:
    {
        NET_DVR_VEHICLE_CONTROL_ALARM struVehicleControlAlarmInfo;
        printf("Vehicle control alarm\n");
        memcpy(&struVehicleControlAlarmInfo, pAlarmInfo, sizeof(NET_DVR_VEHICLE_CONTROL_ALARM));
        break;
    }
    
    
    case COMM_ITS_PLATE_RESULT:
    {
        NET_ITS_PLATE_RESULT struITSPlateResult = {0};
        memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));
       
        for (int i = 0; i < struITSPlateResult.dwPicNum; i++)

        {
            printf("License Plate Number: %s\n", struITSPlateResult.struPlateInfo.sLicense); 
            BYTE* absTime = struITSPlateResult.struPicInfo[i].byAbsTime;
            std::stringstream absTimeStr;    
            // day
            absTimeStr << absTime[6];
            absTimeStr << absTime[7];
            absTimeStr << "-";
            // month
            absTimeStr << absTime[4];
            absTimeStr << absTime[5];
            absTimeStr << "-";
            // year
            absTimeStr << absTime[0];
            absTimeStr << absTime[1];
            absTimeStr << absTime[2];
            absTimeStr << absTime[3];
            absTimeStr << " ";
            // hour
            absTimeStr << absTime[8];
            absTimeStr << absTime[9];
            absTimeStr << ":";
            // minute   
            absTimeStr << absTime[10];
            absTimeStr << absTime[11];
            absTimeStr << ":";
            // second
            absTimeStr << absTime[12];
            absTimeStr << absTime[13];
            
            printf("first pic time: %s\n", absTimeStr.str().c_str());
            unsigned char* plateImageBuf = nullptr;
            // License platethumbnails
            if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0) && (struITSPlateResult.struPicInfo[i].byType == 0))

            {
                plateImageBuf = struITSPlateResult.struPicInfo[i].pBuffer;
                addAlarmResultView(
                table,
                nullptr,
                plateImageBuf,
                struITSPlateResult.struPlateInfo.sLicense,
                absTimeStr.str());
                printf("Plate image buffer number %d\n", i);

            }
        
        }
        break;

    }
    default:
        break;
    }
}

#endif