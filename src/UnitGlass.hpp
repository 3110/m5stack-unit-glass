#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <esp_log.h>

class UnitGlass {
public:
    enum class Register
    {
        CLEAR = 0x00,
        SHOW = 0x10,
        DRAW_STRING = 0x20,
        DRAW_POINT = 0x30,
        DRAW_LINE = 0x40,
        DRAW_CIRCLE = 0x50,
        SCREEN_MODE = 0x60,  // Invert
        DISPLAY_ON_OFF = 0x70,
        STRING_BUFFER = 0x80,
        IMAGE_BUFFER = 0x90,  // Picture Buffer
        COLOR_INVERT = 0xA0,
        DRAW_PICTURE = 0xB0,
        BUZZ = 0xC0,
        BUZZ_ON_OFF = 0xC3,
        KEY_A = 0xD0,
        KEY_B = 0xD1,
        FIRMWARE_VERSION = 0xFE
    };

    enum class ScreenMode
    {
        FRONT_DISPLAY = 0,
        FRONT_DISPLAY_FLIP_180 = 1,
        REVERSE_DISPLAY = 2,
        REVERSE_DISPLAY_FLIP_180 = 3,
    };

    enum class DrawMode
    {
        CLEAR,
        FILL,
    };

    enum class Key
    {
        A,
        B,
    };

    enum class FontSize
    {
        FONT8 = 8,
        FONT16 = 16,
    };

    static const uint8_t I2C_ADDRESS = 0x3D;

    static const uint8_t MAX_WIDTH = 128;
    static const uint8_t MAX_HEIGHT = 64;
    static const size_t MAX_STRING_BUFFER_SIZE = 64;
    static const size_t MAX_IMAGE_BUFFER_SIZE = 1024;

    UnitGlass(void);
    virtual ~UnitGlass(void);

    virtual bool begin(TwoWire& wire, const uint8_t addr, const uint8_t sda,
                       const uint8_t scl);

    virtual bool getFirmwareVersion(uint8_t& version);

    virtual bool clear(void);
    virtual bool show(void);
    virtual bool setScreenMode(ScreenMode mode);
    virtual bool setDisplay(bool on);
    virtual bool invert(bool on);

    virtual bool drawString(const char* str, uint8_t x, uint8_t y,
                            FontSize size = FontSize::FONT8,
                            DrawMode mode = DrawMode::FILL);
    virtual bool drawPixel(uint8_t x, uint8_t y,
                           DrawMode mode = DrawMode::FILL);
    virtual bool drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                          DrawMode mode = DrawMode::FILL);
    virtual bool drawCircle(uint8_t x0, uint8_t y0x, uint8_t r,
                            DrawMode mode = DrawMode::FILL);
    virtual bool drawImage(const uint8_t* buf, size_t len, uint8_t x, uint8_t y,
                           uint8_t width, uint8_t height,
                           DrawMode mode = DrawMode::FILL);

    virtual bool isPressed(Key key);

    virtual bool buzz(uint16_t freq, uint8_t duty, unsigned long duration);

protected:
    virtual bool setBuzzerData(uint16_t freq, uint8_t duty);
    virtual bool setBuzzer(bool on);

    virtual bool setBuffer(Register reg, const uint8_t* buf, size_t len);
    virtual bool setStringBuffer(const char* str);
    virtual bool setImageBuffer(const uint8_t* buf, size_t len);

    virtual bool write(Register reg, uint8_t v, unsigned long wait = 0UL);
    virtual bool writeBytes(Register reg, const uint8_t* buf, uint8_t len,
                            unsigned long wait = 0UL);
    virtual bool read(Register reg, uint8_t& v, bool sendStop = false);
    virtual bool readBytes(Register reg, uint8_t* buf, uint8_t len,
                           bool sendStop = false);

private:
    TwoWire* _wire;
    uint8_t _addr;
};