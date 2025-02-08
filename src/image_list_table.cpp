#include "image_list_table.hpp"

#include <iostream>

ImageListTable::ImageListTable(int X, int Y, int W, int H, const char *L)
    : Fl_Table_Row(X, Y, W, H, L)
{

    // Set up the table
    rows(0);
    cols(3);
    col_header(1);
    col_resize(1);

    // Set column widths
    col_width(0, IMAGE_SIZE + 2 * MARGIN);           // First image
    col_width(1, IMAGE_SIZE + 2 * MARGIN);           // Second image
    col_width(2, W - 2 * (IMAGE_SIZE + 2 * MARGIN)); // Text

    // Set row height
    row_height_all(IMAGE_SIZE + 2 * MARGIN);

}

void ImageListTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H)
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
        const char *headers[] = {"Image 1", "Image 2", "Description"};
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
    int X, int Y, int W, int H, int MARGIN, int IMAGE_SIZE, Fl_Image& image)
{
    // Scale image to fit cell height while maintaining aspect ratio
    int img_h = H - 2 * MARGIN;
    float scale = (float)img_h / image.h();
    int img_w = image.w() * scale;
    int img_x = X + (W - img_w) / 2;
    int img_y = Y + MARGIN;
    image.draw(img_x, img_y, img_w, img_h);


}

void ImageListTable::draw_data(int R, int C, int X, int Y, int W, int H)
{

    if (R >= (int)items.size())
        return;

    fl_push_clip(X, Y, W, H);

    // Draw cell background
    fl_color(FL_WHITE);
    fl_rectf(X, Y, W, H);

    const ListItem &item = items[R];

    switch (C)
    {
    case 0: // First image
        // Scale image to fit cell height while maintaining aspect ratio
        drawImage(X, Y, W, H, MARGIN, IMAGE_SIZE, item.vehicleImage);
        break;


    case 1: // Second image
        drawImage(X, Y, W, H, MARGIN, IMAGE_SIZE, item.plateImage);
        break;


    case 2: // Text
        fl_color(FL_BLACK);
        fl_draw(item.plateText.c_str(), X + MARGIN, Y, W - 2 * MARGIN, H, FL_ALIGN_LEFT | FL_ALIGN_CENTER);
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