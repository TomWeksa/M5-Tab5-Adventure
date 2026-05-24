#include <Arduino.h>
#include <M5Unified.h>

namespace {

constexpr uint32_t kTouchReportIntervalMs = 100;

bool touchActive = false;
uint32_t lastTouchReportAt = 0;

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return M5.Display.color565(r, g, b);
}

void drawHomeScreen() {
    auto& display = M5.Display;
    const uint16_t background = rgb(8, 12, 20);

    display.fillScreen(background);
    display.setTextDatum(textdatum_t::middle_center);
    display.setTextColor(TFT_WHITE, background);
    display.setFont(&fonts::Font4);
    display.drawString("M5 Tab5 Starter", display.width() / 2, display.height() / 2 - 26);

    display.setFont(&fonts::Font2);
    display.setTextColor(rgb(150, 180, 200), background);
    display.drawString("Touch the screen to print coordinates.", display.width() / 2, display.height() / 2 + 24);
    display.setTextDatum(textdatum_t::top_left);
}

void drawTouchPoint(const lgfx::touch_point_t& point) {
    auto& display = M5.Display;
    const int32_t y = display.height() - 48;
    const uint16_t panel = rgb(4, 24, 32);

    display.fillRect(0, y, display.width(), 48, panel);
    display.setFont(&fonts::Font2);
    display.setTextColor(TFT_WHITE, panel);
    display.setCursor(18, y + 14);
    display.printf("Touch x:%d y:%d", point.x, point.y);
}

}  // namespace

void setup() {
    Serial.begin(115200);
    delay(50);
    Serial.println();
    Serial.println("[tab5 starter] boot");

    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setRotation(1);
    M5.Display.setBrightness(180);
    M5.Display.setTextWrap(false);
    drawHomeScreen();
}

void loop() {
    M5.update();

    lgfx::touch_point_t point;
    const uint8_t touchCount = M5.Display.getTouchRaw(&point, 1);
    if (touchCount > 0) {
        M5.Display.convertRawXY(&point, 1);

        const uint32_t now = millis();
        if (!touchActive || now - lastTouchReportAt >= kTouchReportIntervalMs) {
            Serial.printf("[touch] x=%d y=%d\n", point.x, point.y);
            drawTouchPoint(point);
            lastTouchReportAt = now;
        }
        touchActive = true;
    } else if (touchActive) {
        touchActive = false;
        drawHomeScreen();
    }

    delay(10);
}
