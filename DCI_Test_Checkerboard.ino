#include <SPI.h>

const int CS1_PIN = 7;    // LCD CHIP SELECT
const int A0_PIN = 8;     // A0 INSTRUCTION/WRITE DISPLAY
const int BKL_PIN = 9;    // BACKLIGHT ON/OFF
const int RST_PIN = 10;   // RESET (ACTIVE LOW)

// ARDUINO SPI HARDWAR (MOSI=D11, MISO=D12, CLK=D13)

// TEST IMAGE ARRAYS
uint32_t new_image[8][33];

void lcd_write_inst(uint8_t cmd) {
  digitalWrite(CS1_PIN, HIGH);  // CHIP SELECT ON
  digitalWrite(A0_PIN, LOW);    // A0 Low For Instructions
  SPI.transfer(cmd);            // Write command to LCD
  digitalWrite(CS1_PIN, LOW);   // CHIP SELECT OFF
}

void lcd_write_data(uint32_t data) {
  digitalWrite(CS1_PIN, HIGH);  // CHIP SELECT ON
  digitalWrite(A0_PIN, HIGH);   // A0 High for Image Data
  SPI.transfer((data >> 24) & 0xFF);  // Split into 4 8-bit transfers (MSB first)
  SPI.transfer((data >> 16) & 0xFF);
  SPI.transfer((data >> 8) & 0xFF);
  SPI.transfer(data & 0xFF);
  digitalWrite(CS1_PIN, LOW);   // CHIP SELECT OFF
}

void lcd_setup_normal_mode() {
  lcd_write_inst(0x2F);   // Power On
  lcd_write_inst(0xA2);   // LCD BIAS 1/9
  lcd_write_inst(0xA4);   // Entire Display On "Normal"
  lcd_write_inst(0xAF);   // Display On
  lcd_write_inst(0xE7);   // Driver On
  lcd_write_inst(0x81);   // Assert EVR Mode
  lcd_write_inst(0x04);   // Set EVR Register (Exits EVR Mode)
  lcd_write_inst(0xA1);   // Segment Direction "Normal" (DRS claims reveresed on datasheet)
  lcd_write_inst(0xC0);   // Common Driver "Normal"
  lcd_write_inst(0xA6);   // Display Inverse "Normal" (not inverse)
}

void lcd_update_page_address(uint8_t page_index) {
  lcd_write_inst(0x40);                 // Intial Display Line Reset
  lcd_write_inst(0xB0 | page_index);    // Page Address
  lcd_write_inst(0x10);                 // Column Address High Reset
  lcd_write_inst(0x00);                 // Column Address Low Reset
}

void write_page(const uint32_t pageData[33], uint8_t page_num) {
  // update page address
  lcd_update_page_address(page_num);
  // for each 32-bit word
  for (int i = 0; i < 33; i++) {
    // write lcd data
    lcd_write_data(pageData[i]);
  }
}

void write_image(const uint32_t image[8][33]) {
  // for each page (8 total)
  for (uint8_t page = 0; page < 8; page++) {
    // write new page
    write_page(image[page], page);
  }
}

void load_image_data() {
  for (int p = 0; p < 8; p++) {
    for (int i = 0; i < 33; i++) {
      if (i % 2 == 0) {
        new_image[p][i] = 0xF0F0F0F0;
      } else {
        new_image[p][i] = 0x0F0F0F0F;
      }
    }
  }
}


void load_image_data2() {
  for (int p = 0; p < 8; p++) {
    for (int i = 0; i < 33; i++) {
      if (i % 2 == 0) {
        new_image[p][i] = 0x0F0F0F0F;
      } else {
        new_image[p][i] = 0xF0F0F0F0;
      }
    }
  }
}


void setup() {
  // DISCRETE OUTPUT PINS
  pinMode(CS1_PIN, OUTPUT);
  pinMode(A0_PIN, OUTPUT);
  pinMode(BKL_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(CS1_PIN, LOW);   // CHIP SELECT OFF
  digitalWrite(A0_PIN, LOW);    // DEFAULT TO INSTRUCTION
  digitalWrite(BKL_PIN, LOW);   // BACKLIGHT OFF
  digitalWrite(RST_PIN, LOW);   // HOLD IN RESET

  // CONFIGURE SPI HARDWARE
  SPI.begin();
  SPI.beginTransaction(SPISettings(
    250000,     // clock in bps
    MSBFIRST,   // Bit order
    SPI_MODE0   // Clock polarity/phase
  ));
  
  delay(500);   // SHORT PAUSE TO ASSERT RESET SIGNAL

  digitalWrite(RST_PIN, HIGH);   // CLEAR RESET
  lcd_setup_normal_mode();      // SETUP LCD IN NORMAL MODE
  digitalWrite(BKL_PIN, HIGH);   // BACKLIGHT ON

  delay(500);   // SHORT PAUSE TO ENABLE DISPLAY DRIVER
}

void loop() {
  // Write first image
  load_image_data();
  write_image(new_image);
  delay(500);
  // Write the next image
  load_image_data2();
  write_image(new_image);
  delay(500);
}

