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

void BuildEmptyImage() {
  for (int page = 0; page < 8; page++) {
    for (int i = 0; i < 33; i++) {
      new_image[page][i] = 0x00000000;
    }
  }
}


void BuildKevinImage(uint8_t padding)
{
  // Limit padding so 15 data words always fit inside 33 cells
  if (padding > 18) {
    padding = 18;
  }

  // Kevin Source Data: 8 pages, 15 words each
  static const uint32_t source[8][15] = {
    {
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00010307,
      0x07030301, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    {
      0x00000000, 0x00000001, 0x03070606, 0x06060E3E, 0xF2E08000,
      0x000080C0, 0xE070381C, 0x0F070100, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    },
    {
      0x00000000, 0x000000FE, 0xFF330100, 0x00000000, 0x00000000,
      0x00000000, 0x00010302, 0x02C2F27E, 0x3E7EFCC0, 0xC0E07038,
      0x1C0E0703, 0x01010000, 0x00000000, 0x00000000, 0x00000000
    },
    {
      0x00000000, 0x00000000, 0xC3F73E18, 0x18080808, 0x08080818,
      0x10101010, 0x101090F0, 0x00000000, 0x01010101, 0x0303070F,
      0x112020E0, 0xF1FEC0C0, 0xC0C0C0C0, 0xC0E07F1F, 0x00000000
    },
    {
      0x00000000, 0x00033FFC, 0xC0000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000080, 0x80808080, 0x80808080,
      0x80808080, 0x00000000, 0x00000808, 0x0C1EFBF1, 0x00000000
    },
    {
      0x000F1F30, 0x60E0F80E, 0x03000070, 0xF8C8C870, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000010, 0x10180808, 0x183CFCC6, 0x03010000
    },
    {
      0x0F1FB0F0, 0x604040C0, 0x80010101, 0x03030306, 0x06060C0C,
      0x0C0C0C0C, 0x0C0C0C0C, 0x0C181818, 0x18183030, 0x30606060,
      0x60303030, 0x30303018, 0x18181818, 0x18181838, 0xF0C00000
    },
    {
      0x00C0C060, 0x606060C0, 0xC0808080, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
      0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
    }
  };

  // Empty all pages
  for (int page = 0; page < 8; page++) {
    for (int i = 0; i < 33; i++) {
      new_image[page][i] = 0x00000000;
    }
  }
  
  // Copy Kevin (15-word pages) into image after padding
  for (int page = 0; page < 8; page++) {
    for (int i = 0; i < 15; i++) {
      new_image[page][padding + i] = source[page][i];
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
  BuildKevinImage(4);
  write_image(new_image);
  delay(250);
  
  BuildKevinImage(8);
  write_image(new_image);
  delay(250);

  BuildKevinImage(12);
  write_image(new_image);
  delay(250);

  BuildKevinImage(16);
  write_image(new_image);
  delay(250);

  BuildEmptyImage();
  write_image(new_image);
  delay(250);
}

