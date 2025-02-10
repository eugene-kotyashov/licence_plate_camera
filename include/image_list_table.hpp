#pragma once

#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/fl_draw.H>
#include <vector>
#include <string>
#include <memory>
#include <iostream>


struct ListItem {

    Fl_Image& plateImage;
    std::string plateText;
    std::string firstPicTimeStr;
    int index;


    ListItem( Fl_Image& plateImage, const std::string& plateText, const std::string& firstPicTimeStr, int index) :
        
        plateImage(plateImage),
        plateText(plateText),
        firstPicTimeStr(firstPicTimeStr),
        index(index)
    {
    }




    static const unsigned char* LoadJPEGToBuffer(const std::string& fileName) {
        FILE *file = fopen(fileName.c_str(), "rb");
        if (!file) {
            return nullptr;
        }
        
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        unsigned char* buffer = new unsigned char[size];
        fread(buffer, 1, size, file);
        fclose(file);
        return buffer;
    }

};




class ImageListTable : public Fl_Table_Row {
protected:
    std::vector<std::reference_wrapper<ListItem>> items;
     static const int IMAGE_SIZE = 100;  // Size of images in pixels
    static const int MARGIN = 2;       // Cell margin
    void draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) override;

    void draw_header(int X, int Y, int W, int H, const char* title);
    void draw_data(int R, int C, int X, int Y, int W, int H);

public:
   
    ImageListTable(int X, int Y, int W, int H, const char* L = 0);
    
    void addItem(ListItem& item);
    void clearItems();
    int getItemCount() const { return items.size(); }

}; 