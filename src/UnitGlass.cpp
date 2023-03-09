#include "UnitGlass.hpp"

static const char* TAG = "UnitGlass";

UnitGlass::UnitGlass(void) : _wire(nullptr), _addr(0) {
}

UnitGlass::~UnitGlass(void) {
}

bool UnitGlass::begin(TwoWire& wire, const uint8_t addr, const uint8_t sda,
                      const uint8_t scl) {
    this->_wire = &wire;
    this->_addr = addr;

    this->_wire->begin(sda, scl);
    delay(10);
    uint8_t version = 0;
    if (getFirmwareVersion(version)) {
        ESP_LOGI(TAG, "Firmware Version: %d", version);
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to get firmware version");
        return false;
    }
}

bool UnitGlass::getFirmwareVersion(uint8_t& version) {
    return read(Register::FIRMWARE_VERSION, version, true);
}

bool UnitGlass::clear(void) {
    return write(Register::CLEAR, 1);
}

bool UnitGlass::show(void) {
    return write(Register::SHOW, 1);
}

bool UnitGlass::setScreenMode(ScreenMode mode) {
    return write(Register::SCREEN_MODE, static_cast<uint8_t>(mode), 1UL);
}

bool UnitGlass::setDisplay(bool on) {
    return write(Register::DISPLAY_ON_OFF, on ? 1 : 0, 1000UL);
}

bool UnitGlass::invert(bool on) {
    return write(Register::COLOR_INVERT, static_cast<uint8_t>(on ? 1 : 0), 1UL);
}

bool UnitGlass::drawString(const char* str, uint8_t x, uint8_t y, FontSize size,
                           DrawMode mode) {
    const uint8_t data[] = {x, y, static_cast<uint8_t>(size),
                            static_cast<uint8_t>(mode)};
    if (!setStringBuffer(str)) {
        return false;
    }
    return writeBytes(Register::DRAW_STRING, data, sizeof(data), 2UL);
}

bool UnitGlass::drawPixel(uint8_t x, uint8_t y, DrawMode mode) {
    const uint8_t data[] = {x, y, static_cast<uint8_t>(mode)};
    return writeBytes(Register::DRAW_POINT, data, sizeof(data));
}

bool UnitGlass::drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                         DrawMode mode) {
    const uint8_t data[] = {x1, y1, x2, y2, static_cast<uint8_t>(mode)};
    return writeBytes(Register::DRAW_LINE, data, sizeof(data));
}

bool UnitGlass::drawCircle(uint8_t x0, uint8_t y0, uint8_t r, DrawMode mode) {
#if 0
    const uint8_t data[] = {x0, y0, r, static_cast<uint8_t>(mode)};
    return writeBytes(Register::DRAW_CIRCLE, data, sizeof(data));
#else
    int x = 0;
    int y = r;
    int p = 1 - r;
    while (x <= y) {
        drawPixel(x + x0, y + y0, mode);
        drawPixel(y + x0, x + y0, mode);
        drawPixel(-x + x0, y + y0, mode);
        drawPixel(-y + x0, x + y0, mode);
        drawPixel(-x + x0, -y + y0, mode);
        drawPixel(-y + x0, -x + y0, mode);
        drawPixel(x + x0, -y + y0, mode);
        drawPixel(y + x0, -x + y0, mode);

        ++x;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            --y;
            p += 2 * (x - y) + 1;
        }
    }
    return show();
#endif
}

bool UnitGlass::drawImage(const uint8_t* buf, size_t len, uint8_t x, uint8_t y,
                          uint8_t width, uint8_t height, DrawMode mode) {
#if 0
    const uint8_t data[] = {x, y, width, height, static_cast<uint8_t>(mode)};
    bool result = setImageBuffer(buf, len);
    result |= writeBytes(Register::DRAW_PICTURE, data, sizeof(data));
    return result;
#else
    uint8_t x0 = x;
    uint8_t y0 = y;
    uint8_t b = 0;
    DrawMode notMode =
        mode == DrawMode::CLEAR ? DrawMode::FILL : DrawMode::CLEAR;
    for (size_t pos = 0; pos < len; ++pos) {
        b = buf[pos];
        for (uint8_t i = 0; i < 8; ++i) {
            drawPixel(x0, y0, b & (1 << (8 - (i + 1))) ? mode : notMode);
            ++x0;
            if ((x0 - x) == width) {
                x0 = x;
                ++y0;
            }
        }
    }
    return true;
#endif
}

bool UnitGlass::isPressed(Key key) {
    uint8_t v = 0;
    Register reg;
    if (key == Key::A) {
        reg = Register::KEY_A;
    } else if (key == Key::B) {
        reg = Register::KEY_B;
    } else {
        ESP_LOGE(TAG, "Unknown Key: %d", static_cast<uint8_t>(key));
        return false;
    }
    if (!read(reg, v)) {
        ESP_LOGE(TAG, "Failed to read from register %d",
                 static_cast<uint8_t>(reg));
        return false;
    }
    return v == 0;
}

bool UnitGlass::buzz(uint16_t freq, uint8_t duty, unsigned long duration) {
    bool result = true;
    result |= setBuzzerData(freq, duty);
    result |= setBuzzer(true);
    delay(duration);
    result |= setBuzzer(false);
    return result;
}

bool UnitGlass::setBuzzerData(uint16_t freq, uint8_t duty) {
    const uint8_t data[] = {lowByte(freq), highByte(freq), duty};
    return writeBytes(Register::BUZZ, data, sizeof(data));
}

bool UnitGlass::setBuzzer(bool on) {
    return write(Register::BUZZ_ON_OFF, on ? 1 : 0);
}

bool UnitGlass::setBuffer(Register reg, const uint8_t* buf, size_t len) {
    uint8_t data[3] = {0};
    for (size_t pos = 0; pos < len; ++pos) {
        data[0] = lowByte(pos);
        data[1] = highByte(pos);
        data[2] = buf[pos];
        if (!writeBytes(reg, data, sizeof(data))) {
            return false;
        };
    }
    return true;
}

bool UnitGlass::setStringBuffer(const char* str) {
    const size_t len = strlen(str);
    if (len > MAX_STRING_BUFFER_SIZE) {
        ESP_LOGE(TAG, "Exceeded string buffer size : %d > %d", len,
                 MAX_STRING_BUFFER_SIZE);
        return false;
    }
    return setBuffer(Register::STRING_BUFFER,
                     reinterpret_cast<const uint8_t*>(str), len);
}

bool UnitGlass::setImageBuffer(const uint8_t* buf, size_t len) {
    if (len > MAX_IMAGE_BUFFER_SIZE) {
        ESP_LOGE(TAG, "Exceeded image buffer size : %d > %d", len,
                 MAX_IMAGE_BUFFER_SIZE);
        return false;
    }
    return setBuffer(Register::IMAGE_BUFFER, buf, len);
}

bool UnitGlass::write(Register reg, uint8_t v, unsigned long wait) {
    return writeBytes(reg, &v, 1, wait);
}

bool UnitGlass::writeBytes(Register reg, const uint8_t* buf, uint8_t len,
                           unsigned long wait) {
    this->_wire->beginTransmission(this->_addr);
    this->_wire->write(static_cast<uint8_t>(reg));
    for (uint8_t i = 0; i < len; ++i) {
        this->_wire->write(buf[i]);
    }
    uint8_t ret = this->_wire->endTransmission();
    if (ret > 0) {
        ESP_LOGE(TAG, "Failed to write bytes to the register %d: err = %d",
                 static_cast<uint8_t>(reg), ret);
    }
    if (wait > 0UL) {
        delay(wait);
    }
    return ret == 0;
}

bool UnitGlass::read(Register reg, uint8_t& v, bool sendStop) {
    return readBytes(reg, &v, 1, sendStop);
}

bool UnitGlass::readBytes(Register reg, uint8_t* buf, uint8_t len,
                          bool sendStop) {
    this->_wire->beginTransmission(this->_addr);
    this->_wire->write(static_cast<uint8_t>(reg));
    this->_wire->endTransmission(sendStop);
    this->_wire->requestFrom(this->_addr, len);
    int c = -1;
    for (uint8_t pos = 0; pos < len; ++pos) {
        c = this->_wire->read();
        if (c == -1) {
            ESP_LOGE(TAG,
                     "Failed to read %d bytes from the register %d: pos = %d",
                     len, static_cast<uint8_t>(reg), pos);
            this->_wire->flush();
            return false;
        } else {
            buf[pos] = c;
        }
    }
    return true;
}
