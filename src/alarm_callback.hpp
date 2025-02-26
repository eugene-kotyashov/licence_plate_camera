#ifndef ALARM_CALLBACK_HPP
#define ALARM_CALLBACK_HPP
#include "HCNetSDK.h"
#include "image_list_table.hpp"
#include "coutry_strings.hpp"
#include <FL/Fl_JPEG_Image.H>
#include <unordered_map>
#include <cstdio>
#include <string>
#include <sstream>


const char* const directionStr[] = {
    "Unknown",
    "Up",
    "Down",
    "Bi-directional",
    "Westward",
    "Northward",
    "Eastward",
    "Southward",
    "Other direction"
};


struct DBPlustTable {
 
    ImageListTable* table = nullptr;
    sqlite3* db = nullptr;
    std::unordered_map<std::string, int>* abListMap = nullptr;
};



void addAlarmResultView(
    ImageListTable *table,
    ListItem* item
)

{   
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
    
    switch (lCommand)
    {
    case COMM_ISAPI_ALARM:
    {
       
        printf("Alarm ISAPI\n");
        break;
    }
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

        
        auto len = struITSPlateResult.struPlateInfo.dwXmlLen;
        
        std::cout << "XML plate info length is " << len << std::endl;
        if (len > 0) {
            std::cout << struITSPlateResult.struPlateInfo.pXmlBuf << std::endl;
            std::string tmp(struITSPlateResult.struPlateInfo.pXmlBuf, len);
            std::cout << tmp << std::endl;
        }

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
            // License plate thumbnails -> byType == 0
            if ((struITSPlateResult.struPicInfo[i].dwDataLen != 0) && 
                (struITSPlateResult.struPicInfo[i].byType == 0))

            {
                auto country = struITSPlateResult.struPlateInfo.byCountry;
                std::string countryStr = countryNames[country];
                countryStr += "(" + std::to_string(country) + ")";

                plateImageBuf = struITSPlateResult.struPicInfo[i].pBuffer;
                

                DBPlustTable* dbt = (DBPlustTable*)pUser;
                ImageListTable* table = dbt->table;
                std::string dirStr;
                switch (struITSPlateResult.struPicInfo[i].byPicRecogMode)
                {
                    case 0:
                        dirStr = "Forward";
                        break;
                    case 1: 
                        dirStr = "Backward";
                        break;
                    default:
                        dirStr = "Unknown";
                }
                // 0 - block, 1 - allow, 2 - other
                int listType = 2;
                 
                auto listIt = dbt->abListMap->find(
                    struITSPlateResult.struPlateInfo.sLicense);
                if (listIt != dbt->abListMap->end() ) {
                    listType = listIt->second;
                }
                ListItem* item = ListItem::createListItem(
                    // TODO: get index from somewere else
                    table->getItemCount(),
                    plateImageBuf,
                    struITSPlateResult.struPicInfo[i].dwDataLen,
                    struITSPlateResult.struPlateInfo.sLicense,
                    absTimeStr.str(),
                    countryStr,
                    dirStr
                );

                
                addAlarmResultView(
                    table, item);
                item->insertIntoDatabase(dbt->db);
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