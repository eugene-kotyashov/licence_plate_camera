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

    Fl_Image* plateImage;
    const unsigned char* plateImageJpgBuffer = nullptr;
    size_t plateImageBufferSize = 0;
    std::string plateText;
    std::string firstPicTimeStr;
    int index;
    std::string country;
    std::string moveDirection;


    ListItem(
        
        const unsigned char* plateImageBuf,
        size_t plateImageBufSize,
        const std::string& plateText,
        const std::string& firstPicTimeStr,
        int index,
        const std::string& country,
        const std::string& moveDirection
    ) :
        plateImageJpgBuffer(plateImageBuf),
        plateImageBufferSize(plateImageBufSize),
        plateImage(plateImage),
        plateText(plateText),
        firstPicTimeStr(firstPicTimeStr),
        index(index),
        country(country),
        moveDirection(moveDirection)
       
    {
        plateImage = new Fl_JPEG_Image(
            nullptr, plateImageJpgBuffer);
    }


    static ListItem* createListItem(
        int itemId,
        const unsigned char *plateImageBuf,
        size_t plateImageBufSize,
        const std::string &licensePlate,
        const std::string &firstPicTimeStr,
        const std::string &country,
        const std::string &moveDirection)

    {
    
    Fl_Image *plate = new Fl_PNG_Image("plate.png");

    if (plateImageBuf != nullptr) {
        delete plate;
        plate = new Fl_JPEG_Image(nullptr, plateImageBuf);
        if (plate->fail())
        {
            delete plate;
            printf("Failed to load plate image\n");
            plate = new Fl_PNG_Image("plate.png");
        }
    } else {
        printf("plate image buffer is nullptr\n");
    }

    unsigned char *plateImageBufCopy = 
        new unsigned char[plateImageBufSize];
    memcpy(plateImageBufCopy, plateImageBuf, plateImageBufSize);
   
    ListItem *item = new ListItem(
        plateImageBufCopy,
         plateImageBufSize,
        licensePlate,
        firstPicTimeStr,
        itemId,
        country,
        moveDirection);

    return item;

    }
    
    static bool SaveJPEGToFile(
        const unsigned char* buffer,
         size_t bufferLen,
          const std::string& fileName) {
        FILE *file = fopen(fileName.c_str(), "wb");
        if (!file) {
            return false;
        }
        
        fwrite(buffer, 1, bufferLen, file);
        fclose(file);
        return true;
    }

    static const unsigned char* LoadJPEGToBuffer(
        const std::string& fileName, size_t& size) {
        size = 0;
        FILE *file = fopen(fileName.c_str(), "rb");
        if (!file) {
            return nullptr;
        }
        
        fseek(file, 0, SEEK_END);
        size = ftell(file);
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
        std::cerr 
        << "Failed to prepare statement: " << SQL_INSERT << 
        " " << sqlite3_errstr(sqlite3_extended_errcode(db)) << 
        " " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Bind the data to the SQL statement. Assuming plateImage data is stored as binary/blob.
    unsigned char * imageDataPtr = nullptr;
    size_t imageByteCount = 0;
    if (plateImageBufferSize > 0) {
        ListItem::SaveJPEGToFile(
            plateImageJpgBuffer, plateImageBufferSize, "jpegBuffer.jpg");
        std::cout << "plateImage size: " << imageByteCount << std::endl;    
    } else {
        std::cout << "plateImage is empty" << std::endl;
    }
    sqlite3_bind_blob(
        stmt, 1, plateImageJpgBuffer, plateImageBufferSize, SQLITE_TRANSIENT);
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

static std::vector<ListItem> loadItemsFromDatabase(sqlite3* db) {
    std::vector<ListItem> items;
    sqlite3_stmt* stmt;
    std::string selectAll = {"SELECT * FROM "};
    selectAll += std::string(TABLE_NAME);         
    if (sqlite3_prepare_v2(
        db, selectAll.c_str(),
         -1, &stmt, nullptr
    ) != SQLITE_OK) {
        std::cerr << 
        "loadItemsFromDatabase: failed to prepare statement: " 
        << selectAll << " " << sqlite3_errmsg(db) << std::endl;
        return items;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        unsigned char* imageDataPtr = 
            (unsigned char*)sqlite3_column_blob(stmt, 0);
        size_t imageByteCount = sqlite3_column_bytes(stmt, 0);          
        std::string plateText = (const char*)sqlite3_column_text(stmt, 1);  
        std::string firstPicTimeStr = (const char*)sqlite3_column_text(stmt, 2);        
        int index = sqlite3_column_int(stmt, 3);        
        std::string country = (const char*)sqlite3_column_text(stmt, 4);        
        std::string moveDirection = (const char*)sqlite3_column_text(stmt, 5);        
        
        unsigned char* plateImageBuf = new unsigned char[imageByteCount];        
        memcpy(plateImageBuf, imageDataPtr, imageByteCount);        
        
        ListItem item(plateImageBuf, imageByteCount, plateText, firstPicTimeStr, index, country, moveDirection);        
        items.push_back(item);        
    }

    sqlite3_finalize(stmt);
    return items;               
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