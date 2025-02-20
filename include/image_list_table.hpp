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

#include <sqlite3.h>

struct ListItem {

    static auto inline TABLE_NAME = "plate_detection_data";
    static auto inline FIELD_PLATE_IMAGE = "plateImage";
    static auto inline FIELD_PLATE_TEXT = "plateText";
    static auto inline FIELD_FIRST_PIC_TIME_STR = "firstPicTimeStr";
    static auto inline FIELD_INDEX = "idx";
    static auto inline FIELD_COUNTRY = "country";
    static auto inline FIELD_MOVE_DIRECTION = "moveDirection";

    
    static std::string inline  SQL_CREATE_TABLE = 
        "CREATE TABLE IF NOT EXISTS " + std::string(TABLE_NAME) + " ("
        + FIELD_PLATE_IMAGE + " BLOB, "
        + FIELD_PLATE_TEXT + " TEXT, "
        + FIELD_FIRST_PIC_TIME_STR + " TEXT, "
        + FIELD_INDEX + " INTEGER, "
        + FIELD_COUNTRY + " TEXT, "
        + FIELD_MOVE_DIRECTION + " TEXT);";

    static std::string inline SQL_INSERT =
        "INSERT INTO " + std::string(TABLE_NAME) +
        " (" + FIELD_PLATE_IMAGE + ", " + FIELD_PLATE_TEXT +
        ", " + FIELD_FIRST_PIC_TIME_STR + ", " + FIELD_INDEX +
        ", " + FIELD_COUNTRY + ", " + FIELD_MOVE_DIRECTION +
        ") VALUES (?, ?, ?, ?, ?, ?);";

    Fl_Image& plateImage;
    std::string plateText;
    std::string firstPicTimeStr;
    int index;
    std::string country;
    std::string moveDirection;


    ListItem(
        
        Fl_Image& plateImage,
        const std::string& plateText,
        const std::string& firstPicTimeStr,
        int index,
        const std::string& country,
        const std::string& moveDirection
    ) :
        
        plateImage(plateImage),
        plateText(plateText),
        firstPicTimeStr(firstPicTimeStr),
        index(index),
        country(country),
        moveDirection(moveDirection)
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


bool insertIntoDatabase(sqlite3* db) {
    if (!db) return false;

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(
        db, SQL_INSERT.c_str(), -1, &stmt, nullptr
    ) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Bind the data to the SQL statement. Assuming plateImage data is stored as binary/blob.
    char * imageDataPtr = nullptr;
    size_t imageByteCount = 0;
    if (plateImage.count() > 0) {
        imageDataPtr = (char*)plateImage.data();
        imageByteCount = plateImage.w()*plateImage.h()*plateImage.d();      
    }
    sqlite3_bind_blob(stmt, 1, imageDataPtr, imageByteCount, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, plateText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, firstPicTimeStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, index);
    sqlite3_bind_text(stmt, 5, country.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, moveDirection.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
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