#ifndef EXCEL_XLS_EDIT_HPP
#define EXCEL_XLS_EDIT_HPP

#include <xls.h>

class ExcelXlsEdit {
public:
    ExcelXlsEdit(const std::string& filename);
    ~ExcelXlsEdit();

    void addRow(const std::vector<std::string>& row);

private:
    XLDocument doc;
    std::string filename;
};

#endif