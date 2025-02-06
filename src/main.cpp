#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>

#include "camera_device.hpp"

// Callback function for the OK button
void button_cb(Fl_Widget* widget, void*) {
    widget->window()->hide();
}


// Add callback functions for connect and disconnect
void connect_cb(Fl_Widget* widget, void*) {
    // Get the input widget from the window
    Fl_Window* window = widget->window();
    Fl_Input* ip = (Fl_Input*)window->child(0);
    Fl_Int_Input* port = (Fl_Int_Input*)window->child(1);
    const char* ip_value = ip->value();
    int port_value = atoi(port->value());
    // TODO: Implement connection logic using ip_value and port_value
}

void disconnect_cb(Fl_Widget* widget, void*) {
    // TODO: Implement disconnect logic
}

int main(int argc, char *argv[]) {
    // Create a window with more height to accommodate new controls
    CameraDevice camera_device;
    if (!camera_device.isSdkInitialized) {
        Fl_Window error_window(300, 250, "Error");
        error_window.begin();
        Fl_Button ok_button(110, 200, 80, 30, "OK");
        ok_button.callback(button_cb);
        error_window.end();
        error_window.show(argc, argv);
        return Fl::run();
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