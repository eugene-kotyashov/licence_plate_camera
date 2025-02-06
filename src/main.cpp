#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>

// Callback function for the OK button
void button_cb(Fl_Widget* widget, void*) {
    widget->window()->hide();
}

int main(int argc, char *argv[]) {
    // Create a window
    Fl_Window* window = new Fl_Window(300, 200, "FLTK Window");
    window->begin();

    // Create an OK button
    // Position it near the bottom center of the window
    Fl_Button* button = new Fl_Button(110, 150, 80, 30, "OK");
    button->callback(button_cb);

    window->end();
    window->show(argc, argv);

    // Enter the FLTK event loop
    return Fl::run();
} 