#include <M5AtomS3.h>

#include "UnitGlass.hpp"
#include "saito-icon.hpp"

UnitGlass glass;
int count = 0;
char buf[16 + 1] = {0};

void setup(void) {
    M5.begin(true, false, false, false);
    if (!glass.begin(Wire1, UnitGlass::I2C_ADDRESS, 2, 1)) {
        M5.Lcd.println("Failed to initialize Unit Glass");
        while (true) {
            delay(1);
        }
    }

    glass.setScreenMode(UnitGlass::ScreenMode::FRONT_DISPLAY);
    glass.invert(false);
    glass.clear();

    glass.drawImage(SAITO_ICON, sizeof(SAITO_ICON), 32, 0,
                    SAITO_ICON_IMAGE_WIDTH, SAITO_ICON_IMAGE_HEIGHT);
    glass.show();
    delay(1500);

    glass.clear();
    glass.drawString("    M5STACK", 0, 0, UnitGlass::FontSize::FONT16);
    glass.drawString("   Unit Glass", 0, 24, UnitGlass::FontSize::FONT16);
    glass.show();
}

void loop(void) {
    if (glass.isPressed(UnitGlass::Key::A)) {
        glass.buzz(1500, 128, 50UL);
        ++count;
    }
    if (glass.isPressed(UnitGlass::Key::B)) {
        glass.buzz(2300, 128, 50UL);
        --count;
    }
    snprintf(buf, sizeof(buf), "   Count: %-6d", count);
    glass.drawString(buf, 0, 48, UnitGlass::FontSize::FONT16);
    glass.show();
    delay(10);
}
