#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
#define pin_buzzer 15
#define pin_tombol 13


void lcd_i2c(String text = "", int kolom = 0, int baris = 0) {
  // Wire.begin(SDA, SCL);
  byte bar[8] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
  };
  if (text == "") {
    lcd.init();  //jika error pakai lcd.init();
    lcd.backlight();
    lcd.createChar(0, bar);
    lcd.setCursor(0, 0);
    lcd.print("Loading..");
    for (int i = 0; i < 16; i++) {
      lcd.setCursor(i, 1);
      lcd.write(byte(0));
      delay(100);
    }
    delay(50);
    lcd.clear();
  } else {
    lcd.setCursor(kolom, baris);
    lcd.print(text + "                ");
  }
}

void debug(String message, int row = 0, int clear = 1) {
  // Serial.println(message);
  //tampilkan jika menggunakan lcd
  if (clear == 1) {
    lcd.clear();
  }
  lcd.setCursor(0, row);
  lcd.print(message);
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd_i2c();
  pinMode(pin_buzzer, OUTPUT);
  pinMode(pin_tombol, INPUT);
  digitalWrite(pin_buzzer, HIGH);
  delay(1000);
  digitalWrite(pin_buzzer, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  int tombol = digitalRead(pin_tombol);


  if (Serial.available()) {
    String data = Serial.readString();
    
    debug(data);


  } else {

    if (tombol == 0) {
      digitalWrite(pin_buzzer, HIGH);
      delay(100);
      digitalWrite(pin_buzzer, LOW);

      debug("Mengambil gambar");
      Serial.println("0");
      Serial.flush();
      delay(1000);
      Serial.println("1");


    } else {
      digitalWrite(pin_buzzer, LOW);
      debug("AKTIF");
      delay(100);
    }
  }

  delay(1);
}
