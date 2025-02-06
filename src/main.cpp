#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>

#include "camera_device.hpp"
#include "ui_utils.hpp"




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
    const char* ip_value = ip->value();
    int port_value = atoi(port->value());
    CameraDevice* camera_device = (CameraDevice*)camera_device_ptr;
    camera_device->connect(ip_value, port_value, "admin", "123456");
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

int main(int argc, char *argv[]) {
    // Create a window with more height to accommodate new controls
    CameraDevice camera_device;
    if (!camera_device.isSdkInitialized) {
        ui::MessageDialog::showError(
            "Failed to initialize camera SDK");
        return 1;
    }
    
    
    Fl_Window window (300, 250, "FLTK Window");
    window.begin();

    // Add IP input field
    Fl_Input ip_input(70, 20, 120, 25, "IP:");
    ip_input.value("127.0.0.1");  // Default value

    // Add Port input field
    Fl_Int_Input port_input(240, 20, 50, 25, "Port:");
    port_input.value("8080");  // Default port value
    
    // Add Connect and Disconnect buttons
    Fl_Button connect_btn(70, 60, 80, 30, "Connect");
    connect_btn.callback(connect_cb, &camera_device);

    Fl_Button disconnect_btn(160, 60, 80, 30, "Disconnect");
    disconnect_btn.callback(disconnect_cb, &camera_device);

    // Move OK button lower
    Fl_Button button(110, 200, 80, 30, "OK");
    button.callback(button_cb, &camera_device);

    window.end();
    window.show(argc, argv);

    

    // Enter the FLTK event loop
    return Fl::run();
} 