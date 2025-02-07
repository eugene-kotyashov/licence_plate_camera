#pragma once

#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_draw.H>
#include <vector>
#include <string>
#include <memory>

struct ListItem {
    std::shared_ptr<Fl_Image> vehicleImage;
    std::shared_ptr<Fl_Image> plateImage;
    std::string plateText;
};


class ImageListTable : public Fl_Table_Row {
protected:
    std::vector<ListItem> items;
     static const int IMAGE_SIZE = 100;  // Size of images in pixels
    static const int MARGIN = 2;       // Cell margin
    void draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) override;
    void draw_header(int X, int Y, int W, int H, const char* title);
    void draw_data(int R, int C, int X, int Y, int W, int H);

public:
   
    ImageListTable(int X, int Y, int W, int H, const char* L = 0);
    
    void addItem(const ListItem& item);
    void clearItems();
    int getItemCount() const { return items.size(); }
}; 