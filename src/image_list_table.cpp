#include "image_list_table.hpp"
#include <Fl/Fl_Shared_Image.H>

#include <iostream>

ImageListTable::ImageListTable(int X, int Y, int W, int H, const char *L)
    : Fl_Table_Row(X, Y, W, H, L)
{

    // Set up the table
    rows(0);
    cols(7);
    col_header(1);
    col_resize(1);

    // Set column widths
    col_width(0, 10 + 2 * MARGIN);           // id
    col_width(1, IMAGE_SIZE + 2 * MARGIN);           // time text
    col_width(2, 2 *IMAGE_SIZE + 2 * MARGIN); // plate image
    col_width(3, IMAGE_SIZE + 2 * MARGIN); // plate text
    col_width(4, IMAGE_SIZE + 2 * MARGIN);
    col_width(5, IMAGE_SIZE + 2 * MARGIN);
    col_width(6, IMAGE_SIZE + 2 * MARGIN);

    // Set row height
    row_height_all(IMAGE_SIZE + 2 * MARGIN);

}

void ImageListTable::draw_cell(
    TableContext context,
     int R, int C, int X, int Y, int W, int H)
{
    switch (context)
    {
    case CONTEXT_CELL:
    {
        
        draw_data(R, C, X, Y, W, H);
        break;
    }
    case CONTEXT_COL_HEADER:
    {
        const char *headers[] = {
            ListItem::FIELD_INDEX,
             "detection time",
             ListItem::FIELD_PLATE_IMAGE,
             ListItem::FIELD_PLATE_TEXT,
            ListItem::FIELD_COUNTRY,
             ListItem::FIELD_MOVE_DIRECTION,
             ListItem::FIELD_PLATE_LIST_TYPE};
        draw_header(X, Y, W, H, headers[C]);
        break;
    }

    default:
        break;
    }
}

void ImageListTable::draw_header(int X, int Y, int W, int H, const char *title)
{
    fl_push_clip(X, Y, W, H);
    fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
    fl_color(FL_BLACK);
    fl_draw(title, X, Y, W, H, FL_ALIGN_CENTER);
    fl_pop_clip();
}


void drawImage(
    int X, int Y, int W, int H, int MARGIN, int IMAGE_SIZE, Fl_JPEG_Image* image)
{
    double imgWHRatio = double(image->w()) / double(image->h());
    double cellWHRatio = double(W) / double(H);
    double scale = 1.0;
    int yoffset = 0;
    int xoffset = 0;
    if (imgWHRatio > cellWHRatio) {
        scale = double(W) / double(image->w()); // fit width
        yoffset = ceil(0.5*abs(double(H)-scale*double(image->h())));
    } else { 
        scale = double(H) / double(image->h()); // fit height
        xoffset = ceil(0.5*abs(double(W)-scale*double(image->w())));
    }
    int img_h = ceil(scale*double(image->h())); 
    int img_w = ceil(scale*double(image->w()));
    Fl_Image *rgb = image->copy(img_w, img_h);
    rgb->draw(X+xoffset, Y+yoffset);
    
   
   
    // std::cout << "drawing " << shared->w() << " " << shared->h() << ": " << W  <<" " << H  <<" " << std::endl;

}

void ImageListTable::draw_data(int R, int C, int X, int Y, int W, int H)
{

    if (R >= (int)items.size())
        return;

    fl_push_clip(X, Y, W, H);

    // Draw cell background
    fl_color(FL_WHITE);
    fl_rectf(X, Y, W, H);

    auto  drawString = [=](const char* str) {
        fl_color(FL_BLACK);
        fl_draw(
            str, X + MARGIN,
            Y, W - 2 * MARGIN, H, FL_ALIGN_LEFT | FL_ALIGN_CENTER);
    };

    const ListItem &item = items[R];

    switch (C)
    {
    case 0: // index
        drawString( std::to_string(item.index).c_str());
        break;
    case 3: // plate text
        drawString(item.plateText.c_str());
        break;

    case 2: // plate image
    {
        drawImage(X, Y, W, H, MARGIN, IMAGE_SIZE, item.plateImage);
    }
        break;
    case 1: // time text
        drawString(item.firstPicTimeStr.c_str());
        break;
    case 4: // country
       drawString(item.country.c_str());
        break;
    case 5: // move direction
        drawString(item.moveDirection.c_str());
        break;
    case 6: // plate list type
        drawString(item.plateListType.c_str());
        break;

    default:
        break;

    }

    // Draw cell border
    fl_color(FL_LIGHT2);
    fl_rect(X, Y, W, H);

    fl_pop_clip();
}

void ImageListTable::addItem(ListItem& item)
{
    items.push_back(item);
    rows(items.size());
    row_height_all(IMAGE_SIZE + 2 * MARGIN);
    redraw();
}

void ImageListTable::clearItems()
{
    items.clear();
    rows(0);
    redraw();
}