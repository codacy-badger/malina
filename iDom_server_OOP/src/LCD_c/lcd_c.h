#ifndef LCD_C_H
#define LCD_C_H

#include <iostream>
#include <string>
#include <regex>
#include "../liquidcrystal_i2c-for-ardupi/LiquidCrystal_I2C.h"
#include "../idom_api.h"

class LCD_c : public iDom_API
{
public:
    LCD_c(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows);
    ~LCD_c();
    void init();
    void set_lcd_STATE (int i);
    void set_print_song_state (int i);
    void song_printstr();
    void printRadioName(bool clear, int col, int row, const std::string& st);
    void noBacklight();
    void clear();
    void printString(bool clear, int col, int row, const std::string& str);
    void printVolume (int vol);

    LiquidCrystal_I2C main_lcd;
    void printSongName (const std::string& songName);
    void checkState();
    bool m_play_Y_N = false;
    std::string getData();
    std::string dump() const;
private:
    //std::string intToStr(int n);
    int m_lcd_state = 0;
    int m_print_song_state = 0;
    int m_rePrint = 0;
    std::string m_row1;
    std::string m_row2;
    std::string m_radioName;

};

#endif // LCD_C_H
