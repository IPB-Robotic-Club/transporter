#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Initialize the I2C LCD at address 0x27, size 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Design a trophy as a custom character
byte trophy[8] = {
    B00100,
    B01110,
    B11111,
    B11111,
    B01110,
    B11111,
    B11111,
    B00100};

void setup()
{
    lcd.begin();
    lcd.backlight();

    // Create the custom trophy character at location 0
    lcd.createChar(0, trophy);

    // Display initial text
    lcd.clear();
    lcd.setCursor(2, 0); // Set cursor position for "AGROTRANSPORTER"
    lcd.print("AGROTRANSPORTER");
    lcd.setCursor(6, 1); // Set cursor position for "IS THE WINNER"
    lcd.print("IS THE WINNER");

    // Initial delay
    delay(1000);
}

void loop()
{
    // Trophy animation movement
    for (int pos = 0; pos <= 15; pos++)
    {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("AGROTRANSPORTER");
        lcd.setCursor(6, 1);
        lcd.print("IS THE WINNER");

        // Display the trophy moving on the second row
        lcd.setCursor(pos, 1);
        lcd.write(byte(0)); // Draw the trophy at position "pos"

        delay(150); // Animation speed
    }

    // Delay after the animation ends
    delay(500);
}
