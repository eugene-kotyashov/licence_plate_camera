#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Secret_Input.H>  // For password field
#include <FL/Fl_PNG_Image.H>
#include <filesystem>

#include "camera_device.hpp"
#include "ui_utils.hpp"
#include "image_list_table.hpp"
#include <thread>

#define TABLE_CONTROL_ID 11

std::vector<ListItem> plateDetectionData;

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

    Fl_Window* window = widget->window();
    ImageListTable* table = (ImageListTable*)window->child(TABLE_CONTROL_ID);


    camera_device->enableArming(table);


    ui::MessageDialog::show("ANPR enable successful");

}

void test_jpeg_cb(Fl_Widget* widget, void* camera_device_ptr) { 
    Fl_Window* window = widget->window();
    ImageListTable* table = (ImageListTable*)window->child(TABLE_CONTROL_ID);
    auto veh = new Fl_JPEG_Image(nullptr, ListItem::LoadJPEGToBuffer("vehicle.jpg"));
    auto plate = new Fl_JPEG_Image(nullptr, ListItem::LoadJPEGToBuffer("plate.jpg"));

    ListItem* item = new  ListItem(*veh, *plate, "jpeg nuff");
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
    Fl_Button anpr_btn(70, 170, 170, 30, "Configure ANPR");
    anpr_btn.callback(configure_anpr_cb, &camera_device);

    Fl_Button test_jpeg_btn(250, 170, 170, 30, "Test jpeg buffer");
    test_jpeg_btn.callback(test_jpeg_cb, &camera_device);

    // Add Stop ANPR button
    Fl_Button stop_anpr_btn(430, 170, 170, 30, "Stop ANPR");
    stop_anpr_btn.callback([](Fl_Widget*, void* v) {
        auto* camera = static_cast<CameraDevice*>(v);
        camera->disableArming();
    }, &camera_device);

    // Add Download Block List button
    Fl_Button download_blocklist_btn(610, 170, 170, 30, "Download Block List");
    download_blocklist_btn.callback(download_blocklist_cb, &camera_device);

    // Add Upload Block List button - add after download button
    Fl_Button upload_blocklist_btn(610, 130, 170, 30, "Upload Block List");
    upload_blocklist_btn.callback(upload_blocklist_cb, &camera_device);

    // Add the table
    ImageListTable table(20, 220, 760, 320, "Detection Results");
    
    //if (!item.vehicleImage.fail() && !item.plateImage.fail()) {  // Only add if both images loaded successfully
    Fl_PNG_Image* vehicleImage = new Fl_PNG_Image("vehicle.png");
    Fl_PNG_Image* plateImage = new Fl_PNG_Image("plate.png");

	for (int i = 0; i < 1; i++) {

		ListItem item(
			*vehicleImage,
			*plateImage,
			"License Plate: ABC123"
		);
		table.addItem(item);
	}

    window.end();
    window.show(argc, argv);
    
    if (Fl::lock() != 0) {
        std::cout << "FLTK is not compiled with threading support." << std::endl;
    }
    return Fl::run();
} 