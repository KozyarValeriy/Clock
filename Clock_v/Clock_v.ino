#include <Wire.h>                          // Билиотека для рабоыт по шине I2C

#include "RTClib.h"
RTC_DS3231 rtc;
DateTime now;

#include <SPI.h>
#include <Adafruit_BME280.h>               // Бислиотека метио датчика
#include <LiquidCrystal_I2C.h>             // Библиотека LCD дисплея

#define SensTime 10000                     // Время обновления показаний датчика
#define Button_1 7                         // Кнопка переключения времени/даты 
#define Button_2 12                        // Кнопка переключения показаний датчика
#define Button_3 10                        // Кнопка включения/выключения подсветки
#define Buzzer 4                           // Подключение бипера
#define RESET_CLOCK 0 

#define SEALEVELPRESSURE_HPA (1013.25)     // для датчика давления


#include <GyverTimer.h>
GTimer_ms sensorsTimer(SensTime);
GTimer_ms drawSensorsTimer(SensTime);
GTimer_ms clockTimer(500);
GTimer_ms predictTimer((long)10 * 60 * 1000);         // 10 минут

//-----------------------Задание переменных-----------------------

static const char *days[] = {
          "Mon", 
          "Tue", 
          "Wed", 
          "Thu", 
          "Fri", 
          "Sat", 
          "Sun"};                                  // Так как датчик времени глючит с выводом дней недели

static const char *mnth[] = {
          "Jan",
          "Feb",
          "Mar",
          "Apr",
          "May",
          "Jun",
          "Jul",
          "Aug",
          "Sep",
          "Oct",
          "Nov",
          "Dec"};  

Adafruit_BME280 bme;                       // Установка датчика давления
LiquidCrystal_I2C lcd(0x27,16,2);          // Устанавливаем дисплей

int hrs, mins, secs, day_t, month_t, year_t, dayofweek;
byte mode_time = 0;                        // Счетчкик для переключения температуры/давления/высоты/влажности
/*
 0 - Отображение часов на две строки
 1 - Отображение даты + часы + день недели
 2 - Отображение часы + сек + день недели
 3 - Отображение даты + год + месяц
 */
byte mode_sens = 0;                        // Счетчик для переключения режимов отображения времени
/*
 0 - Отображение темп + влаж + давление
 1 - Обображение темп до десятых + влажность
 2 - Отображение давление + высота
 3 - Отображение темп до десятых + вероятность дождя
 */
boolean flag_buz = 0;                      // Флаг, чтобы бипер пищал только один раз при 00 минутах
boolean flag_button_1 = 0;                 // Флаг переключения кнопки №1
boolean flag_button_2 = 0;                 // Флаг переключения кнопки №2
boolean flag_button_3 = 1;                 // Флаг переключения кнопки №3
boolean flag = 1;                          // Флаг переключения подсветки экрана

float dispTemp;
byte dispHum;
long dispPres;
int dispRain;
int Height;
String zero, zero_m, zero_min, zero_days;
int delta;
uint32_t pressure_array[6];
uint32_t sumX, sumY, sumX2, sumXY;
float a, b;
byte time_array[6];


// Цифры
uint8_t LT[8] = {0b00111,  0b01111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
uint8_t UB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};
uint8_t RT[8] = {0b11100,  0b11110,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
uint8_t LL[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b01111,  0b00111};
uint8_t LB[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};
uint8_t LR[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11110,  0b11100};
uint8_t UMB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};
uint8_t LMB[8] = {0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};

//----------------------------------------------------------------

void drawDig(byte dig, byte x, byte y) {
    switch (dig) {
      case 0:
          lcd.setCursor(x, y); // set cursor to column 0, line 0 (first row)
          lcd.write(0);  // call each segment to create
          lcd.write(1);  // top half of the number
          lcd.write(2);
          lcd.setCursor(x, y + 1); // set cursor to colum 0, line 1 (second row)
          lcd.write(3);  // call each segment to create
          lcd.write(4);  // bottom half of the number
          lcd.write(5);
          break;
      case 1:
          lcd.setCursor(x + 1, y);
          lcd.write(1);
          lcd.write(2);
          lcd.setCursor(x + 2, y + 1);
          lcd.write(5);
          break;
      case 2:
          lcd.setCursor(x, y);
          lcd.write(6);
          lcd.write(6);
          lcd.write(2);
          lcd.setCursor(x, y + 1);
          lcd.write(3);
          lcd.write(7);
          lcd.write(7);
          break;
      case 3:
          lcd.setCursor(x, y);
          lcd.write(6);
          lcd.write(6);
          lcd.write(2);
          lcd.setCursor(x, y + 1);
          lcd.write(7);
          lcd.write(7);
          lcd.write(5);
          break;
      case 4:
          lcd.setCursor(x, y);
          lcd.write(3);
          lcd.write(4);
          lcd.write(2);
          lcd.setCursor(x + 2, y + 1);
          lcd.write(5);
          break;
      case 5:
          lcd.setCursor(x, y);
          lcd.write(0);
          lcd.write(6);
          lcd.write(6);
          lcd.setCursor(x, y + 1);
          lcd.write(7);
          lcd.write(7);
          lcd.write(5);
          break;
      case 6:
          lcd.setCursor(x, y);
          lcd.write(0);
          lcd.write(6);
          lcd.write(6);
          lcd.setCursor(x, y + 1);
          lcd.write(3);
          lcd.write(7);
          lcd.write(5);
          break;
      case 7:
          lcd.setCursor(x, y);
          lcd.write(1);
          lcd.write(1);
          lcd.write(2);
          lcd.setCursor(x + 1, y + 1);
          lcd.write(0);
          break;
      case 8:
          lcd.setCursor(x, y);
          lcd.write(0);
          lcd.write(6);
          lcd.write(2);
          lcd.setCursor(x, y + 1);
          lcd.write(3);
          lcd.write(7);
          lcd.write(5);
          break;
      case 9:
          lcd.setCursor(x, y);
          lcd.write(0);
          lcd.write(6);
          lcd.write(2);
          lcd.setCursor(x + 1, y + 1);
          lcd.write(4);
          lcd.write(5);
          break;
      case 10:
          lcd.setCursor(x, y);
          lcd.write(32);
          lcd.write(32);
          lcd.write(32);
          lcd.setCursor(x, y + 1);
          lcd.write(32);
          lcd.write(32);
          lcd.write(32);
          break;
    }
}

void drawdots(byte x, byte y, boolean state) {
    byte code;
    if (state) code = 165;
    else code = 32;
    lcd.setCursor(x, y);
    lcd.write(code);
    lcd.setCursor(x, y + 1);
    lcd.write(code);
}

void drawdots_min(byte x, boolean state) {
    byte code;
    if (state) code = 58;
    else code = 32;
    lcd.setCursor(x, 0);
    lcd.write(code);
}

void drawClock(byte hours, byte minutes, byte x, byte y) {
    if (hours / 10 == 0) drawDig(10, x, y);
    else drawDig(hours / 10, x, y);
    drawDig(hours % 10, x + 4, y);
    drawDig(minutes / 10, x + 8, y);
    drawDig(minutes % 10, x + 12, y);
}

void loadClock() {
    lcd.createChar(0, LT);
    lcd.createChar(1, UB);
    lcd.createChar(2, RT);
    lcd.createChar(3, LL);
    lcd.createChar(4, LB);
    lcd.createChar(5, LR);
    lcd.createChar(6, UMB);
    lcd.createChar(7, LMB);
}

//----------------Функция первоначальной настройки----------------

void setup() {   
    //Serial.begin(9600);
    pinMode(Button_1, INPUT);
    pinMode(Button_2, INPUT);
    pinMode(Button_3, INPUT_PULLUP);
    pinMode(2,OUTPUT);
    digitalWrite(2, HIGH);
    bme.begin();                   // Включение датчика давления
	
  	bme.setSampling(Adafruit_BME280::MODE_FORCED,
                      Adafruit_BME280::SAMPLING_X1, // temperature
                      Adafruit_BME280::SAMPLING_X1, // pressure
                      Adafruit_BME280::SAMPLING_X1, // humidity
                      Adafruit_BME280::FILTER_OFF);
	
    lcd.init();                    // Включение экрана 
    lcd.backlight();               // Включение подсветки экрана
    lcd.clear();
    rtc.begin();                  // Включение датчика времени

    if (RESET_CLOCK || rtc.lostPower())
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    
    now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
    day_t = now.day();
    month_t = now.month();
    year_t = now.year();
    dayofweek = now.dayOfTheWeek();
    /*
    Serial.print("Секунды: ");
    Serial.println(secs);
    Serial.print("Минуты: ");
    Serial.println(mins);
    Serial.print("Часы: ");
    Serial.println(hrs);
    Serial.print("День: ");
    Serial.println(day_t);
    Serial.print("Месяц: ");
    Serial.println(mnth[month_t - 1]);
    Serial.print("Год: ");
    Serial.println(year_t);
    Serial.print("День недели: ");
    Serial.println(days[now.dayOfTheWeek() - 1]);
    */
    
  	if (hrs < 10) zero = "0";
  	else zero = "";
  
    if (month_t < 10) zero_m = "0";
    else zero_m = "";
  
    if (mins < 10) zero_min = "0";
    else zero_min = "";

    if (day_t < 10) zero_days = "0";
    else zero_days = "";
    
    loadClock();
    drawClock(hrs, mins, 0, 0);
  	readSensors();
  	
  	for (byte i = 0; i < 6; i++) {   			// счётчик от 0 до 5
    		pressure_array[i] = dispPres;  			// забить весь массив текущим давлением
    		time_array[i] = i;             			// забить массив времени числами 0 - 5
        predictRain();
    }
	
}

//----------------------Основной цикл работы----------------------

void loop() { 

    if (sensorsTimer.isReady()) {
        readSensors();   						// читаем показания датчиков с периодом SENS_TIME  
    		if (mode_time != 0) drawSensors();
	  }
	
    if (clockTimer.isReady()) clockTick();      // два раза в секунду пересчитываем время и мигаем точками 
	  if (predictTimer.isReady()) predictRain();	
    
    Button1Click();
    Button2Click();
    Button3Click();    
}
