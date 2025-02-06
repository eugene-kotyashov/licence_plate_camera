#ifndef UI_UTILS_HPP
#define UI_UTILS_HPP

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <string>


namespace ui {

class MessageDialog {
public:
    static void show(
        const std::string& message,
        const std::string& title = "Message") {

        Fl_Window* dialog = new Fl_Window(300, 150, title.c_str());
        dialog->begin();

        // Message box with word wrap
        Fl_Box* box = new Fl_Box(10, 10, 280, 90);
        box->copy_label(message.c_str());
        box->align(FL_ALIGN_WRAP | FL_ALIGN_CENTER);

        // OK button
        Fl_Button* button = new Fl_Button(110, 110, 80, 30, "OK");
        button->callback([](Fl_Widget* w, void*) {
            w->window()->hide();
        });

        dialog->end();
        dialog->set_modal();
        dialog->show();
    }

    static void showError(const std::string& message) {
        show(message, "Error");
    }
};

} // namespace ui 
#endif // UI_UTILS_HPP