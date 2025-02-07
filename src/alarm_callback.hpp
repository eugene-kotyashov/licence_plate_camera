#ifndef ALARM_CALLBACK_HPP
#define ALARM_CALLBACK_HPP
#include "HCNetSDK.h"
#include "image_list_table.hpp"
#include <cstdio>

void CALLBACK GetLicencePlatePicsAndText(
    LONG lCommand,
    NET_DVR_ALARMER *pAlarmer,
    char *pAlarmInfo,
    DWORD dwBufLen,
    void *pUser)
{
    int i = 0;
    char filename[100];
    ImageListTable* table = (ImageListTable*)pUser;
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
        printf("License Plate Number: %s\n", struPlateResult.struPlateInfo.sLicense); // License plate number
        switch (struPlateResult.struPlateInfo.byColor)                                // License plate color
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
        // Scene picture
        if (struPlateResult.dwPicLen != 0 && struPlateResult.byResultType == 1)
        {
           
        }
        // License plate picture
        if (struPlateResult.dwPicPlateLen != 0 && struPlateResult.byResultType == 1)
        {
        }
        // Processing other data...
        break;
    }
    case COMM_ITS_PLATE_RESULT:
    {
        NET_ITS_PLATE_RESULT struITSPlateResult = {0};
        memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));
        for (i = 0; i < struITSPlateResult.dwPicNum; i++)
        {
            printf("License Plate Number: %s\n", struITSPlateResult.struPlateInfo.sLicense); // License plate number
            switch (struITSPlateResult.struPlateInfo.byColor)                                // License plate color
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
            // Save scene picture
            if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0) && (struITSPlateResult.struPicInfo[i].byType == 1) ||
                (struITSPlateResult.struPicInfo[i].byType == 2))
            {
                
                //fwrite(struITSPlateResult.struPicInfo[i].pBuffer, struITSPlateResult.struPicInfo[i].dwDataLen, 1, fSnapPic);
                
            }
            // License plate thumbnails
            if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0) && (struITSPlateResult.struPicInfo[i].byType == 0))
            {
                
            }
            // Processing other data...
        }
        break;
    }
    default:
        break;
    }
}

#endif