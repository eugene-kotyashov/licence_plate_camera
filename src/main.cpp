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

// Add this helper function at the top of the file
std::shared_ptr<Fl_Image> loadImage(const char* path) {
    std::shared_ptr<Fl_PNG_Image> img(new Fl_PNG_Image(path));
    if (img->fail()) {
        ui::MessageDialog::showError(std::string("Failed to load image: ") + path);
        return nullptr;
    }
    return img;
}

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

    camera_device->enableArming();

    ui::MessageDialog::show("ANPR enable successful");
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
    password_input.value("123456");
    
    // Add Connect and Disconnect buttons
    Fl_Button connect_btn(70, 130, 80, 30, "Connect");
    connect_btn.callback(connect_cb, &camera_device);

    Fl_Button disconnect_btn(160, 130, 80, 30, "Disconnect");
    disconnect_btn.callback(disconnect_cb, &camera_device);

    // Add ANPR Config button
    Fl_Button anpr_btn(70, 170, 170, 30, "Configure ANPR");
    anpr_btn.callback(configure_anpr_cb, &camera_device);

    // Add the table
    ImageListTable* table = new ImageListTable(20, 220, 760, 320, "Detection Results");
    
    // Example of adding items (you would do this in response to events)
    ListItem item;
    
    // Load images with error checking
    auto img1 = loadImage("vehicle.png");
    auto img2 = loadImage("plate.png");
    
    if (!img1 || !img2) {
        ui::MessageDialog::showError("Failed to load one or more images. Current directory: " + 
            std::string(std::filesystem::current_path().string()));
        return  Fl::run();
    }
    
    item.image1 = img1;
    item.image2 = img2;
    
    if (item.image1 && item.image2) {  // Only add if both images loaded successfully
        item.text = "License Plate: ABC123";
        for (int i = 0; i < 10; i++) {
            table->addItem(item);
        }

    }

    // Move OK button to bottom
    Fl_Button button(360, 550, 80, 30, "OK");
    button.callback(button_cb, &camera_device);

    window.end();
    window.show(argc, argv);

    return Fl::run();
} 