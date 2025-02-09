#ifndef ALARM_CALLBACK_HPP
#define ALARM_CALLBACK_HPP
#include "HCNetSDK.h"
#include "image_list_table.hpp"
#include <FL/Fl_JPEG_Image.H>
#include <cstdio>

void addAlarmResultView(
    ImageListTable *table,
    const unsigned char *vehicleImageBuf,
    const unsigned char *plateImageBuf,
    const std::string &licensePlate)

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
   
    ListItem *item = new ListItem(*veh, *plate, licensePlate);
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
    int i = 0;
   
    ImageListTable* table = (ImageListTable* )pUser;
    switch (lCommand)
    {
    case COMM_ALARM:
    {
        NET_DVR_ALARMINFO struAlarmInfo;
        memcpy(&struAlarmInfo, pAlarmInfo, sizeof(NET_DVR_ALARMINFO));
        switch (struAlarmInfo.dwAlarmType)
        {
        case 3: // Motion detection alarm

            for (i = 0; i < 16; i++) // #define MAX_CHANNUM 16 //The maximum number of channels
            {
                if (struAlarmInfo.dwChannel[i] == 1)
                {
                    printf("Channel Number with Motion Detection Alarm %d\n", i + 1);
                }
            }
            break;
        default:
            break;
        }
        break;
    }
    case COMM_UPLOAD_PLATE_RESULT:
    {
        NET_DVR_PLATE_RESULT struPlateResult = {0};
        
        memcpy(&struPlateResult, pAlarmInfo, sizeof(struPlateResult));
        printf("License Plate Number: %s\n", struPlateResult.struPlateInfo.sLicense);
        switch (struPlateResult.struPlateInfo.byColor)                                

        {
        case VCA_BLUE_PLATE:
            printf("Vehicle Color: Blue\n");
            break;
        case VCA_YELLOW_PLATE:
            printf("Vehicle Color: Yellow\n");
            break;
        case VCA_WHITE_PLATE:
            printf("Vehicle Color: White\n");
            break;
        case VCA_BLACK_PLATE:
            printf("Vehicle Color: Black\n");
            break;
        default:
            break;
        }
        const unsigned char* vehicleImageBuf = nullptr;
        const unsigned char* plateImageBuf = nullptr;
        // Scene picture
        if (struPlateResult.dwPicLen != 0 && struPlateResult.byResultType == 1)

        {
            printf("Vehicle image buffer %d\n", i);
            vehicleImageBuf =
                (const unsigned char*)struPlateResult.pBuffer1;

        }

        // License plate picture
        if (struPlateResult.dwPicPlateLen != 0 && struPlateResult.byResultType == 1)
        {
            printf("plate image buffer %d\n", i);
            plateImageBuf =
                (const unsigned char*)struPlateResult.pBuffer1;
        }
        

        addAlarmResultView(
            table,
             vehicleImageBuf,
              plateImageBuf,
               struPlateResult.struPlateInfo.sLicense);



        break;

    }
    case COMM_ITS_PLATE_RESULT:
    {
        NET_ITS_PLATE_RESULT struITSPlateResult = {0};
        memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));
        for (i = 0; i < struITSPlateResult.dwPicNum; i++)
        {
            printf("License Plate Number: %s\n", struITSPlateResult.struPlateInfo.sLicense); 
            switch (struITSPlateResult.struPlateInfo.byColor)                               
            {
            case VCA_BLUE_PLATE:
                printf("Vehicle Color: Blue\n");
                break;
            case VCA_YELLOW_PLATE:
                printf("Vehicle Color: Yellow\n");
                break;
            case VCA_WHITE_PLATE:
                printf("Vehicle Color: White\n");
                break;
            case VCA_BLACK_PLATE:
                printf("Vehicle Color: Black\n");
                break;
            default:
                break;
            }
            unsigned char* vehicleImageBuf = nullptr;
            unsigned char* plateImageBuf = nullptr;
            // Save scene picture
            if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0) && (struITSPlateResult.struPicInfo[i].byType == 1) ||
                (struITSPlateResult.struPicInfo[i].byType == 2))

            {
                vehicleImageBuf = struITSPlateResult.struPicInfo[i].pBuffer;
                printf("Vehicle image buffer number %d\n", i);
            }
            // License plate thumbnails
            if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0) && (struITSPlateResult.struPicInfo[i].byType == 0))

            {
                plateImageBuf = struITSPlateResult.struPicInfo[i].pBuffer;
                printf("Plate image buffer number %d\n", i);
            }
            

            addAlarmResultView(
                table,
                vehicleImageBuf,
                plateImageBuf,
                struITSPlateResult.struPlateInfo.sLicense);
            
            // Processing other data...
        }
        break;

    }
    default:
        break;
    }
}

#endif