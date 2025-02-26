#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Secret_Input.H>  // For password field
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Choice.H>  // For combo box
#include <filesystem>

#include "camera_device.hpp"
#include "ui_utils.hpp"
#include "image_list_table.hpp"
#include <thread>


constexpr int GATE_CHOICE_ID = 11;
constexpr int TABLE_CONTROL_ID = 15;

constexpr int OUTPUT_ID = 1;

std::vector<ListItem> plateDetectionData;


DBPlustTable dataView;

sqlite3* db = nullptr;

// Callback function for the OK button

void button_cb(Fl_Widget* widget, void*) {
    widget->window()->hide();
}

// Add callback functions for connect and disconnect
void connect_cb(Fl_Widget* widget, void* camera_device_ptr) {
    // Get the input widget from the window
    Fl_Window* window = widget->window();
    Fl_Input* ip = (Fl_Input*)window->child(0);
    Fl_Int_Input* port = (Fl_Int_Input*)window->child(1);
    Fl_Input* username = (Fl_Input*)window->child(2);
    Fl_Secret_Input* password = (Fl_Secret_Input*)window->child(3);

    const char* ip_value = ip->value();
    int port_value = atoi(port->value());
    const char* username_value = username->value();
    const char* password_value = password->value();

    CameraDevice* camera_device = (CameraDevice*)camera_device_ptr;
    camera_device->connect(
        ip_value, port_value,
         username_value, password_value);
    if (camera_device->loggedUserId < 0) {
        ui::MessageDialog::showError(
            "Failed to connect to camera " +
             std::to_string(camera_device->lastError));
    } else {
        ui::MessageDialog::show("Successfully connected to camera");
    }
}


void disconnect_cb(Fl_Widget* widget, void* camera_device_ptr) {
    // TODO: Implement disconnect logic
    CameraDevice* camera_device = (CameraDevice*)camera_device_ptr;
    camera_device->disconnect();
    if (camera_device->loggedUserId >= 0) {
        ui::MessageDialog::showError(
            "Failed to disconnect from camera " +
             std::to_string(camera_device->lastError));
    }
}

void configure_anpr_cb(Fl_Widget* widget, void* camera_device_ptr) {
    CameraDevice* camera_device = (CameraDevice*)camera_device_ptr;
    
    if (camera_device->loggedUserId < 0) {
        ui::MessageDialog::showError("Please connect to camera first");
        return;
    }

    if (camera_device->enableArming(&dataView)) {

        ui::MessageDialog::show("ANPR enable successful");
    } else {
        ui::MessageDialog::showError(
            "arming enable error " 
            + std::to_string(camera_device->lastError));
    }

}

void test_jpeg_cb(Fl_Widget* widget, void* dbPtr) { 
    Fl_Window* window = widget->window();

    ImageListTable* table = dataView.table;
    if (table == nullptr) {
        std::cerr << "table is null" << std::endl;
        return;
    }
    size_t bufSize = 0;
    const unsigned char* buf = 
        ListItem::LoadJPEGToBuffer("plate.jpg", bufSize);
    static int recId = 1000;
    ListItem* item = new  ListItem(
        buf, bufSize,  "TEST_PLATE", "12:00:00", recId++, "US", "forward");
    
    item->insertIntoDatabase(db);
    table->addItem( *item);
}

// Add new callback function
void download_blocklist_cb(Fl_Widget* widget, void* camera_device_ptr) {
    CameraDevice* camera_device = (CameraDevice*)camera_device_ptr;
    
    if (camera_device->loggedUserId < 0) {
        ui::MessageDialog::showError("Please connect to camera first");
        return;
    }

    if (camera_device->startDownloadVehicleBlockAllowList()) {
        ui::MessageDialog::show("Started downloading vehicle block list");
    } else {
        ui::MessageDialog::showError(
            "Failed to start downloading vehicle block list. Error: " + 
            std::to_string(camera_device->lastError));
    }
}

// Add new callback function after download_blocklist_cb
void upload_blocklist_cb(Fl_Widget* widget, void* camera_device_ptr) {
    CameraDevice* camera_device = (CameraDevice*)camera_device_ptr;
    
    if (camera_device->loggedUserId < 0) {
        ui::MessageDialog::showError("Please connect to camera first");
        return;
    }

    // Template file path
    const char* templateFile = "black_white_list_upload.xls";
    
    if (!std::filesystem::exists(templateFile)) {
        ui::MessageDialog::showError("Template file not found: " + std::string(templateFile));
        return;
    }
    auto uploadHandle = 
        camera_device->startUploadVehicleBlockAllowList(templateFile);
    if (uploadHandle != -1) {
        ui::MessageDialog::show("Started uploading vehicle block list");
        std::thread uploadStatusMonitor([uploadHandle]() {
            DWORD uploadProgress = 0; // 0...100 range
            while (uploadProgress < 100 ) {
                LONG uploadStatus = NET_DVR_GetUploadState(
                    uploadHandle, &uploadProgress);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (uploadStatus == 1) {
                    printf("Upload finished\n");
                    break;
                } else if (uploadStatus == 2) {
                    printf("Upload progress: %d\n", uploadProgress);
                    
                } else {
                    printf("Upload failed with error: %d\n", uploadStatus);
                    break;
                }
            }

        });

        uploadStatusMonitor.detach();

    } else {
        ui::MessageDialog::showError(
            "Failed to upload vehicle block list. Error: " + 
            std::to_string(camera_device->lastError));
    }

}

// Add new callback function
void control_gate_cb(Fl_Widget* widget, void* v) {
    auto* window = widget->window();
    auto* camera = static_cast<CameraDevice*>(v);
    
    // Get the combo box value
    Fl_Choice* gateChoice = (Fl_Choice*)window->child(GATE_CHOICE_ID);  // Gate control combo box
    int selectedGate = gateChoice->value();
    printf("selectedGate %d\n", selectedGate);
    if (camera->loggedUserId < 0) {
        ui::MessageDialog::showError("Please connect to camera first");
        return;
    }

    BYTE outputStatus = -1;
    if (camera->getAlarmOutputStatus(OUTPUT_ID, outputStatus) < 0) {
        ui::MessageDialog::showError("Failed to get alarm out status");
    }
    if (outputStatus < 0 && outputStatus > 1) {
        ui::MessageDialog::showError(
            "Got wrong value for alarm out status");
    }
    gateChoice->value(outputStatus);
}

int main(int argc, char *argv[]) {
    // Increase window size to accommodate the tableCameraDevice camera_device;

    
    CameraDevice camera_device;
    if (!camera_device.isSdkInitialized) {
        ui::MessageDialog::showError(
            "Failed to initialize camera SDK");
        return  Fl::run();
    }    
    Fl_Window window(800, 600, "Camera Connection");
    window.begin();

    // Add IP input field
    Fl_Input ip_input(70, 20, 120, 25, "IP:");
    ip_input.value("192.168.0.64");

    // Add Port input field
    Fl_Int_Input port_input(240, 20, 50, 25, "Port:");
    port_input.value("8000");

    // Add Username field
    Fl_Input username_input(70, 55, 220, 25, "User:");
    username_input.value("admin");

    // Add Password field
    Fl_Secret_Input password_input(70, 90, 220, 25, "Pass:");
    password_input.value("Neolink79");
    
    // Add Connect and Disconnect buttons
    Fl_Button connect_btn(70, 130, 80, 30, "Connect");
    connect_btn.callback(connect_cb, &camera_device);

    Fl_Button disconnect_btn(160, 130, 80, 30, "Disconnect");
    disconnect_btn.callback(disconnect_cb, &camera_device);

    // Add ANPR Config button
    Fl_Button anpr_btn(250, 170, 170, 30, "Configure ANPR");
    anpr_btn.callback(configure_anpr_cb, &camera_device);

    Fl_Button test_jpeg_btn(430, 170, 170, 30, "Test jpeg buffer");
    test_jpeg_btn.callback(test_jpeg_cb, nullptr);

    // Add Stop ANPR button
    Fl_Button stop_anpr_btn(610, 170, 170, 30, "Stop ANPR");
    stop_anpr_btn.callback([](Fl_Widget*, void* v) {
        CameraDevice* camera = static_cast<CameraDevice*>(v);
        
        if ( camera->disableArming()) {
            ui::MessageDialog::show("ANPR disabled successfully");
        } else {
            ui::MessageDialog::showError(
                "Failed to disable ANPR. Error: " + 
                std::to_string(camera->lastError));
        }
    }, &camera_device);

    // Add Download Block List button
    Fl_Button download_blocklist_btn(610, 130, 170, 30, "Download Block List");
    download_blocklist_btn.callback(download_blocklist_cb, &camera_device);

    // Add Upload Block List button - add after download button
    Fl_Button upload_blocklist_btn(610, 130, 170, 30, "Upload Block List");
    upload_blocklist_btn.callback(upload_blocklist_cb, &camera_device);

    // Add Gate Control combo box and button
    Fl_Choice* gateChoice = new Fl_Choice(70, 170, 80, 25, "Status:");
    gateChoice->add("0");
    gateChoice->add("1");
    gateChoice->value(0);  // Set default selection

    Fl_Button gate_control_btn(160, 170, 80, 30, "Get Alout");
    gate_control_btn.callback(control_gate_cb, &camera_device);

    Fl_Button trigger_alarm_btn(160, 210, 80, 30, "Trigger Alout");
    trigger_alarm_btn.callback([](Fl_Widget*, void* v) {
        CameraDevice* camera = static_cast<CameraDevice*>(v);
        if (!camera->triggerAlarmOutput(OUTPUT_ID)) {
            ui::MessageDialog::showError(
                "Failed to trigger alarm output. Error: " + 
                std::to_string(camera->lastError));
        }
    }, &camera_device);

    Fl_Button listen_btn(250, 130, 170, 30, "Get ITC ability");
    listen_btn.callback([](Fl_Widget*, void* v) {
        CameraDevice* camera = static_cast<CameraDevice*>(v);
        if (!camera->getDeviceITCAbility(1)) {
            ui::MessageDialog::showError(
                "Failed to get device ITC ability. Error: " + 
                std::to_string(camera->lastError));
        }

    }, &camera_device);


    Fl_Button get_blocklist_schedule_btn(
        250, 210, 170, 30, "Get Blocklist Schedule");
    get_blocklist_schedule_btn.callback([](Fl_Widget*, void* v) {
        CameraDevice* camera = static_cast<CameraDevice*>(v);
        if (!camera->getVehicleBlockListShedule()) {
            ui::MessageDialog::showError(
                "Failed to get block list schedule. Error: " + 
                std::to_string(camera->lastError));
        } else {
            ui::MessageDialog::show("Got block list schedule");
        }
    }, &camera_device);

    Fl_Button searchLPListAudit_btn(430, 210, 170, 30, "Search LPListAudit");
    searchLPListAudit_btn.callback([](Fl_Widget*, void* v) {
        CameraDevice* camera = static_cast<CameraDevice*>(v);
        std::string searchId = "1234";
        if (!camera->searchLPListAudit(searchId, 0, 10)) {
            ui::MessageDialog::showError(
                "Failed to search LPListAudit. Error: " + 
                std::to_string(camera->lastError));
        } else {
            ui::MessageDialog::show("Searched LPListAudit. Search ID: " + searchId);
        }
    }, &camera_device);

    Fl_Button get_trigger_config_btn(430, 130, 170, 30, "Get Trigger Config");
    get_trigger_config_btn.callback([](Fl_Widget*, void* v) {
        CameraDevice* camera = static_cast<CameraDevice*>(v);
        if (!camera->getTriggerConfig()) {
            ui::MessageDialog::showError(
                "Failed to get trigger config. Error: " + 
                std::to_string(camera->lastError));
        }
    }, &camera_device);

    Fl_Button load_blocklist_btn(610, 210, 170, 30, "Load Block List");
    load_blocklist_btn.callback([](Fl_Widget*, void* v) {
        CameraDevice* camera = static_cast<CameraDevice*>(v);
        if (!camera->loadBlockAllowListCurl("192.168.0.64", 1)) {
            ui::MessageDialog::showError(
                "Failed to load block allow list. Error: " + 
                std::to_string(camera->lastError));
        }
    }, &camera_device);


    // Add the table
    ImageListTable table(20, 270, 760, 320, "Detection Results");
    dataView.table = &table;
    dataView.abListMap = &(camera_device.blockAllowList);
    //if (!item.vehicleImage.fail() && !item.plateImage.fail()) {  // Only add if both images loaded successfully
    Fl_PNG_Image* vehicleImage = new Fl_PNG_Image("vehicle.png");
    Fl_PNG_Image* plateImage = new Fl_PNG_Image("plate.png");

    window.end();
    window.show(argc, argv);


    int rc = sqlite3_open("plate_detection.db", &db);
    if (!std::filesystem::exists("plate_detection.db"))
    {
        sqlite3 *db_temp;
        int rc = sqlite3_open("plate_detection.db", &db_temp);
        if (rc)
        {
            ui::MessageDialog::showError(
                "Failed to create database file. Error: " +
                std::string(sqlite3_errmsg(db_temp)));
        }
        sqlite3_close(db_temp);
    }

    if (rc)
    {
        ui::MessageDialog::showError(
            "Failed to open database file. Error: " +
            std::string(sqlite3_errmsg(db)));
    }

    const char *createTableSql = ListItem::SQL_CREATE_TABLE.data();
    rc = sqlite3_exec(db, createTableSql, nullptr, nullptr, nullptr);
    if (rc)
    {
        ui::MessageDialog::showError(
            "Failed to create table in database file. Error: " +
            std::string(sqlite3_errmsg(db)));
    }

    dataView.db = db;

    std::vector<ListItem> items =
        ListItem::loadItemsFromDatabase(db);
    if (items.empty())
    {
        std::cout << "No items found in database." << std::endl;
    }
    for (auto& item : items)
    {
        table.addItem(item);
    }

    if (Fl::lock() != 0)
    {
        std::cout << "FLTK is not compiled with threading support." << std::endl;
    }
    return Fl::run();
} 