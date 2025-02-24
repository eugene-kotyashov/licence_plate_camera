#ifndef __CAMERA_DEVICE_HPP__
#define __CAMERA_DEVICE_HPP__

#include <string>
#include "../incEn/HCNetSDK.h"
#include "alarm_callback.hpp"
#include "listen_callback.hpp"
#include "image_list_table.hpp"

#include <curl/curl.h>
#include <pugixml.hpp>


#define ISAPI_STATUS_LEN (4096*4)

char listAuditInputXML[] =
     "<LPListAuditSearchDescription version=\"2.0\" xmlns=\"http://www.isapi.org/ver20/XMLSchema\"><searchID></searchID><searchResultPosition></searchResultPosition><maxResults></maxResults><type></type><LicensePlate></LicensePlate><cardNo></cardNo><cardID></cardID></LPListAuditSearchDescription>";

const char descItDeviceAbilityXML[] =
 R"XML(<?xml version="1.0" encoding="utf-8"?><ITDeviceAbility version="2.0"><channelNO></channelNO></ITDeviceAbility>)XML";
std::size_t write_data(void* buf, std::size_t size, std::size_t nmemb,
    void* userp)
{
    if(auto sp = static_cast<std::string*>(userp))
    {
        sp->append(static_cast<char*>(buf), size * nmemb);
        return size * nmemb;
    }

    return 0;
}


// Функция выполнения HTTP-запросов
bool SendHttpRequest(
    const std::string& url,
    const std::string& method,
    std::string* data) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "failed to init curl" << std::endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, "admin:Neolink79");
    // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    
    CURLcode res = curl_easy_perform(curl);
    bool result = false;
    if (res != CURLE_OK) {
        std::cerr << "failed request: " << curl_easy_strerror(res) << std::endl;
        result = false;
    } else {
        std::cout << "request succeded " << method << " " << url << std::endl;
        result = true;
    }

    std::cout << *data << std::endl;

    curl_easy_cleanup(curl);
    return result;
}

bool GetAlarmOutputs(const std::string& CAMERA_IP) {
    std::string url = "http://" + std::string(CAMERA_IP) + "/ISAPI/System/IO/outputs";
    std::string res;
    return SendHttpRequest(url, "GET", &res);
}

// Проверка статуса выхода №1
bool GetAlarmOutputStatus(
    const std::string &CAMERA_IP,
    int outputID,
    BYTE &outputStatus)
{
    std::string url = "http://" +
                      std::string(CAMERA_IP) + "/ISAPI/System/IO/outputs/" +
                      std::to_string(outputID) + "/status";
    std::string statusXMLString;
    bool result =
        SendHttpRequest(url, "GET", &statusXMLString);

    if (result)
    {
        pugi::xml_document doc;
        // Parse the XML from the string
        auto parseResult = doc.load_string(statusXMLString.c_str());

        // Check for parsing errors
        if (!parseResult)
        {
            std::cerr << "XML parsed with errors, error description: "
                      << parseResult.description() << "\n";
            return false;
        }

        // Accessing elements and attributes
        pugi::xml_node root = doc.child("IOPortStatus");
        std::string statusString;
        // std::cout << "root " << root.name() << std::endl;
        for (auto child : root.children())
        {   
            // std::cout << child.name() << std::endl;
            // std::cout << child.text().as_string() << std::endl;
            if (0 == strcmp(child.name(), "ioState"))
            {
                // std::cout << "status string " << child.text().as_string() << std::endl;
                statusString = child.text().as_string();
                break;
            }
        }
        if (statusString.empty())
        {
            std::cerr << "status string not found" << std::endl;
            return false;
        }
        outputStatus = (statusString == "inactive") ? 0 : 1;
        return true;
    }
    return false;
}

// Активация тревожного выхода №1
bool TriggerAlarmOutput(const std::string& CAMERA_IP, int outputID) {
    std::string url = "http://" + std::string(CAMERA_IP) + "/ISAPI/System/IO/outputs/" 
    + std::to_string(outputID) + "/trigger";
    std::string res;
    return SendHttpRequest(url, "PUT", &res);
}


struct CameraDevice
{
    const int MAX_USERNAME_LENGTH = 32;
    const int MAX_PASSWORD_LENGTH = 32;

    bool isSdkInitialized = false;
    int loggedUserId = -1;
    int lastError = 0;
    LONG lHandle = -1;
    LONG listenHandle = -1;

    CameraDevice()

    {
        isSdkInitialized = NET_DVR_Init();
        if (isSdkInitialized)
        {
            NET_DVR_SetLogToFile(3, "sdk_log", true);
        }
    }

    void connect(
        const std::string &ip,
        int port,
        const std::string &username,
        const std::string &password)
    {
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

        if (loggedUserId < 0)
        {
            lastError = NET_DVR_GetLastError();
        }
    }

    void disconnect()
    {
        if (!isSdkInitialized)
            return;
        if (loggedUserId < 0)
            return;
        if (!NET_DVR_Logout(loggedUserId))
        {
            lastError = NET_DVR_GetLastError();
            return;
        }
        loggedUserId = -1;
    }

    bool enableArming(void *table)
    {
        lastError = 0;
        if (!isSdkInitialized)
            return false;
        if (loggedUserId < 0)
            return false;
        NET_DVR_SETUPALARM_PARAM_V50 struSetupParamV50 = {0};
        struSetupParamV50.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM_V50);
        // Alarm information type to upload:
        // 0-History Alarm (NET_DVR_PLATE_RESULT), 1-Real-Time Alarm
        // (NET_ITS_PLATE_RESULT)
        struSetupParamV50.byAlarmInfoType = 1;
        // Arming Level: Level-2 arming (for traffic device)
        struSetupParamV50.byLevel = 0;
        //  set next field to 0 to receive OMM_ALARM_V30, alarm information
        // structure: NET_DVR_ALARMINFO_V30
        // struSetupParamV50.byRetAlarmTypeV40 = 0;
        struSetupParamV50.byRetAlarmTypeV40 = 1;

        char szSubscribe[1024] = {0};
        // The following code is for alarm subscription (subscribe all)
        memcpy(szSubscribe,
               "<SubscribeEvent version=\"2.0\" xmlns=\"http://www.isapi.org/ver20/XMLSchema\">\r\n<eventMode>all</eventMode>\r\n",
               1024);
        lHandle = -1;
        // if (0 == strlen(szSubscribe))
        //{

        lHandle = NET_DVR_SetupAlarmChan_V50(
            loggedUserId, &struSetupParamV50,
             NULL, strlen(szSubscribe));
        //}
        /*else
        {
            // Subscribe
            lHandle = NET_DVR_SetupAlarmChan_V50(loggedUserId, &struSetupParamV50, szSubscribe, strlen(szSubscribe));
        }
        */
        if (lHandle < 0)
        {
            lastError = NET_DVR_GetLastError();
            return false;
        }

        if (!NET_DVR_SetDVRMessageCallBack_V50(
                0, GetLicencePlatePicsAndText, table))
        {
            lastError = NET_DVR_GetLastError();
            return false;
        }

        return true;
    }

    bool disableArming()
    {
        if (!isSdkInitialized)
            return false;
        if (loggedUserId < 0)
            return false;
        if (!NET_DVR_CloseAlarmChan_V30(lHandle))
        {
            lastError = NET_DVR_GetLastError();
            return false;
        }
        NET_DVR_SetDVRMessageCallBack_V50(
            0, nullptr, nullptr);
        lHandle = -1;
        return true;
    }

    int startDownloadVehicleBlockAllowList()
    {
        if (!isSdkInitialized || loggedUserId < 0)
        {
            return -1;
        }

        char *sFileName = "black_white_list_template.xls";

        int downloadHandle = NET_DVR_StartDownload(
            loggedUserId,
            NET_SDK_DOWNLOAD_VEHICLE_BLOCKALLOWLIST_FILE,
            nullptr,
            0,
            sFileName);

        if (downloadHandle < 0)
        {
            lastError = NET_DVR_GetLastError();
        }

        return downloadHandle;
    }

    int startUploadVehicleBlockAllowList(const char *filePath)
    {
        if (!isSdkInitialized || loggedUserId < 0)
        {
            return -1;
        }

        int uploadHandle = NET_DVR_UploadFile_V40(
            loggedUserId,
            UPLOAD_VEHICLE_BLOCKALLOWLIST_FILE,
            nullptr,
            0,
            filePath,
            nullptr,
            0);

        if (uploadHandle < 0)
        {
            lastError = NET_DVR_GetLastError();
        }

        return uploadHandle;
    }

    int setBarrierGateControl(BYTE controlCommand)
    {
        if (!isSdkInitialized || loggedUserId < 0)
        {
            return -1;
        }
        NET_DVR_BARRIERGATE_CFG struBarrierGateCfg = {0};
        struBarrierGateCfg.dwSize = sizeof(NET_DVR_BARRIERGATE_CFG);
        struBarrierGateCfg.byBarrierGateCtrl = controlCommand;
        if (NET_DVR_RemoteControl(
                loggedUserId,
                NET_DVR_BARRIERGATE_CTRL,
                &struBarrierGateCfg,
                sizeof(struBarrierGateCfg)) == false)
        {

            lastError = NET_DVR_GetLastError();
            return -1;
        }
        return 0;
    }

    int checkDeviceITCAbility(DWORD dwAbility)
    {
        if (!isSdkInitialized || loggedUserId < 0)
        {
            return -1;
        }
    }

    int getTriggerConfig()
    {
        if (!isSdkInitialized || loggedUserId < 0)
        {
            return -1;
        }

        NET_DVR_TRIGGER_COND struTriggerCond = {0};
        struTriggerCond.dwSize = sizeof(struTriggerCond);
        struTriggerCond.dwChannel = 0;
        struTriggerCond.dwTriggerMode = ITC_POST_SINGLEIO_TYPE;

        NET_ITC_TRIGGERCFG struItcTriggerCfg = {0};
        struItcTriggerCfg.dwSize = sizeof(struItcTriggerCfg);
        DWORD dwStatus = 0;
        BOOL bRet = FALSE;

        if (!NET_DVR_GetDeviceConfig(
                loggedUserId,
                NET_DVR_GET_TRIGGEREX_CFG,
                1,
                (LPVOID)&struTriggerCond,
                sizeof(struTriggerCond),
                &dwStatus,
                &struItcTriggerCfg,
                sizeof(struItcTriggerCfg)))
        {
            lastError = NET_DVR_GetLastError();
            return -1;
        }
        return 0;
    }

    // Функция запроса состояния тревожного выхода
    int  getAlarmOutputStatus(int outputId, BYTE& outputStatus )
    {
        if (!GetAlarmOutputStatus("192.168.0.64", outputId, outputStatus)) {
            std::cout << "error getting alarm out status " << std::endl;
            return -1;
        }   
        else
        {
            std::cout << "alarm out status ";
            
                std::cout 
                << "alarm out : " 
                << (int)outputStatus << std::endl;
        }
        return 0;
    }

    // Функция активации тревожного выхода
    bool triggerAlarmOutput(int outputNumber)
    {
        if (!TriggerAlarmOutput("192.168.0.64", outputNumber))
        {
            std::cout << "error getting alarm out status " << std::endl;
            return false;
        }
        return true;
    }

    bool startListen()
    {
        lastError = 0;
        if (!isSdkInitialized || loggedUserId < 0)
            return false;
        if ( (listenHandle = NET_DVR_StartListen_V30(
            NULL, 7200, ListenCallback, NULL) ) < 0)
        {
            lastError = NET_DVR_GetLastError();
            return false;
        }
        return true;
    }

    bool stopListen()
    {
        if (!isSdkInitialized || loggedUserId < 0)
            return false;
        if (!NET_DVR_StopListen_V30(listenHandle))
        {
            lastError = NET_DVR_GetLastError();
            return false;
        }
        listenHandle = -1;
        return true;
    }

    bool setTriggerModeForWhiteBlackList()
    {
        if (!isSdkInitialized || loggedUserId < 0)
            return false;
        NET_DVR_TRIGGER_COND triggerCondition = {0};
        triggerCondition.dwSize = sizeof(triggerCondition);
        triggerCondition.dwChannel = 0;
        // TODO: clarify the meaning of this mode
        triggerCondition.dwTriggerMode = ITC_POST_SINGLEIO_TYPE;
        DWORD triggerErrorStatus = 0;
        NET_ITC_TRIGGERCFG triggerCfg = {0};
        triggerCfg.dwSize = sizeof(triggerCfg);
        triggerCfg.struTriggerParam.byEnable = 1;
        /* TODO: ????
        triggerCfg.struTriggerParam.dwTriggerType =
                ITC_POST_SINGLEIO_TYPE;
        triggerCfg.struTriggerParam.
            uTriggerParam.struSingleIO.
            struSingleIO[0].
        */
        if (!NET_DVR_SetDeviceConfig(
                loggedUserId,
                NET_DVR_SET_TRIGGEREX_CFG,
                1,
                &triggerCondition,            
                sizeof(triggerCondition),
                &triggerErrorStatus,                   
                &triggerCfg,
                sizeof(triggerCfg)                  
                )) 
        {
            lastError = NET_DVR_GetLastError();
            return false;
        }


        return true;
    }

    bool getVehicleBlockListShedule()  {
        if (!isSdkInitialized || loggedUserId < 0)
            return false;
        
        NET_DVR_STD_CONFIG struCfg = {0};

        LONG channel = 0;
        
        struCfg.lpCondBuffer = &channel;
        struCfg.dwCondSize = sizeof(channel);
        NET_DVR_EVENT_SCHEDULE struOutShedule = {0};
        struOutShedule.dwSize = sizeof(struOutShedule);
         
        struCfg.lpOutBuffer = &struOutShedule;
        struCfg.dwOutSize = sizeof(struOutShedule);

        char statusBuf[ISAPI_STATUS_LEN] = {0};

        memset(statusBuf, 0, ISAPI_STATUS_LEN);
        struCfg.lpStatusBuffer = statusBuf;
        struCfg.dwStatusSize = ISAPI_STATUS_LEN;

        if (!NET_DVR_GetSTDConfig(
                loggedUserId,
                NET_DVR_GET_VEHICLE_BLOCKLIST_SCHEDULE,
                &struCfg)
            )
        {
            lastError = NET_DVR_GetLastError();
            std::cerr << "failed to get blocklist schedule" << std::endl;
            return false;
        }
        for (int weekDay = 0; weekDay < 7; weekDay++) {
            std::cout << "week day " << weekDay << std::endl;
            for (int schedIntervalIdx = 0; schedIntervalIdx < 8; schedIntervalIdx++) {
                
                std::cout << "sched interval " << schedIntervalIdx << std::endl;
                std::cout << "start hour " <<
                std::to_string(struOutShedule.struAlarmTime[weekDay][schedIntervalIdx].byStartHour)
                << " start minute " <<
                std::to_string(struOutShedule.struAlarmTime[weekDay][schedIntervalIdx].byStartMin)
                << " end hour " <<
                std::to_string(struOutShedule.struAlarmTime[weekDay][schedIntervalIdx].byStopHour)
                << " end minute " <<
                std::to_string(struOutShedule.struAlarmTime[weekDay][schedIntervalIdx].byStopMin)
                << std::endl;
            }
        }
        return true;

    }

    bool searchLPListAudit(std::string &searchId, int startPosition, int maxResults)
    {
        if (!isSdkInitialized || loggedUserId < 0)
            return false;
        NET_DVR_XML_CONFIG_INPUT reqIn = {0};
        NET_DVR_XML_CONFIG_OUTPUT reqOut = {0};
        char url[256] = "POST /ISAPI/Traffic/channels/1/searchLPListAudit";
        reqIn.dwSize = sizeof(reqIn);
        reqOut.dwSize = sizeof(reqOut);
        reqIn.dwRecvTimeOut = 30000;
        reqIn.lpRequestUrl = url;
        reqIn.dwRequestUrlLen = strlen(url);
        DWORD dwInBufferLen = 1024 * 1024;
        char *pInBuffer = new char[dwInBufferLen];
        char *pStatusBuffer = new char[dwInBufferLen];
        memset(pInBuffer, 0, dwInBufferLen);
        memset(pStatusBuffer, 0, dwInBufferLen);

        auto cleanup = [pInBuffer, pStatusBuffer]()
        {
            delete[] pInBuffer;
            delete[] pStatusBuffer;
        };

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_buffer(listAuditInputXML, strlen(listAuditInputXML));
        if (result.status !=  pugi::status_ok )
        {
            std::cerr << "failed to parse xml" << result.description() << std::endl;
            cleanup();
        }
        auto searchIDNode = doc.child("LPListAuditSearchDescription").child("searchID");
        searchIDNode.text().set(searchId.c_str());

        auto searchResultPositionNode = doc.child("LPListAuditSearchDescription").child("searchResultPosition");
        searchResultPositionNode.text().set(std::to_string(startPosition).c_str());

        auto maxResultsNode = doc.child("LPListAuditSearchDescription").child("maxResults");
        maxResultsNode.text().set(std::to_string(maxResults).c_str());

        std::stringstream ss;
        doc.save(ss);
        
        
       
        memcpy(pInBuffer, ss.str().c_str(), ss.str().length());
        std::cout << "listAuditInputXML" << std::endl;
        std::cout << pInBuffer << std::endl;    
        reqIn.lpInBuffer = pInBuffer;
        reqIn.dwInBufferSize = ss.str().length();

        reqOut.lpStatusBuffer = pStatusBuffer;
        reqOut.dwStatusSize = dwInBufferLen;
        if (!NET_DVR_STDXMLConfig(loggedUserId, &reqIn, &reqOut))
        {
            lastError = NET_DVR_GetLastError();
            cleanup();
            return false;
        }
        std::cout << "LP Audit Search Result:" << std::endl;
        std::cout << reqOut.lpOutBuffer << std::endl;
        cleanup();
        return true;
    }


    bool getDeviceITCAbility(int channelNo) {
        if (!isSdkInitialized || loggedUserId < 0)
            return false;
        DWORD dwAbitlityType = DEVICE_ABILITY_INFO;
    
        int inBufSize = 512;
        int outBufSize = 1024*024;
        char* inBuf = new char[inBufSize];
        char* outBuf = new char[outBufSize];
        auto cleanup = [inBuf, outBuf]()
        {
            if (inBuf != nullptr )    
                delete[] inBuf;
            if (outBuf != nullptr )   
                delete[] outBuf;
        };

        pugi::xml_document doc;
        pugi::xml_parse_result result = 
        doc.load_buffer(descItDeviceAbilityXML,
             strlen(descItDeviceAbilityXML));
        if (result.status !=  pugi::status_ok )
        {
            std::cerr << "failed to parse xml" <<
                 result.description() << std::endl;
            cleanup();
        }
        pugi::xml_node channelNoNode = 
        doc.child("ITDeviceAbility").child("channelNO");
        if (channelNoNode)
        {
            channelNoNode.text().set(std::to_string(channelNo).c_str());
        }
        memset( inBuf, 0, inBufSize);
        std::stringstream ss;   
        doc.save(ss);
        memcpy(inBuf, ss.str().c_str(), ss.str().length());

        std::cout << "DEVICE_ABILITY_INFO request" << std::endl;
        std::cout << inBuf << std::endl;
        if (!NET_DVR_GetDeviceAbility(
            loggedUserId, DEVICE_ABILITY_INFO,
            inBuf, inBufSize, outBuf, outBufSize
        )) {
            lastError = NET_DVR_GetLastError();
            cleanup();
            return false;
        }

        std::cout << outBuf << std::endl;
        cleanup();
        return true;
    }

    ~CameraDevice()
    {
        if (isSdkInitialized)
        {

            NET_DVR_Cleanup();
        }
    }
};



#endif // __CAMERA_DEVICE_HPP__
