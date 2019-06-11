boolean dotFlag;
void clockTick() {
    dotFlag = !dotFlag;
	  if (dotFlag) {              // каждую секунду пересчёт времени
		    secs++;     
		    if (secs > 59) {        // каждую минуту
    		    secs = 0;
    			  mins++;
    			  if (mins <= 59){
                if (mins < 10) zero_min = "0";
                else zero_min = "";
                drawClock();
			      }           
		    }
    		if (mins > 59) {      // каждый час
      			now = rtc.now();
      			secs = now.second();
      			mins = now.minute();
      			hrs = now.hour();
      			day_t = now.day();
      			month_t = now.month();
      			year_t = now.year();
      			dayofweek = now.dayOfTheWeek();				
      			if (mins == 0) {
        				if (flag_buz == 1) {
          					tone(Buzzer, 3000, 40);
          					flag_buz = 0;
        				}
      			} 
    		else flag_buz = 1;			
    		if (hrs > 23) hrs = 0;    			
    		if (hrs < 10) zero = "0";
    		else zero = "";   
        if (month_t < 10) zero_m = "0";
        else zero_m = "";
        if (day_t < 10) zero_days = "0";
        else zero_days = "";
  			drawClock();
        }
        if (mode_time == 2) {
          	lcd.setCursor(8, 0);
          	if (secs < 10) lcd.print("0");             
          	lcd.print(secs); 
      		  }
    }
  	if (mode_time == 0) drawdots(7, 0, dotFlag);
  	if (mode_time == 1) drawdots_min(8, dotFlag);
  	if (mode_time == 2) {
        drawdots_min(4, dotFlag);
        drawdots_min(7, dotFlag);
    }
}

void readSensors() {
    bme.takeForcedMeasurement();
    dispTemp = bme.readTemperature();
    dispHum = bme.readHumidity();
    dispPres = bme.readPressure();
    Height = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void drawClock() {
    switch(mode_time) {
        case (0):
            Clear_screen(0);
            Clear_screen(1);
            drawClock(hrs, mins, 0, 0);
            break;
        case (1): 
            Clear_screen(0);
            lcd.setCursor(0, 0);
            lcd.print(zero_days + String(day_t) + "." + zero_m + String(month_t) + " " + zero + String(hrs) + ":" + zero_min + String(mins) + " " + String(days[dayofweek - 1]));
            break;
        case (2):
            Clear_screen(0); 
            lcd.setCursor(2, 0);            
            lcd.print(zero + String(hrs) + ":" + zero_min + String(mins) + ":" + String(secs));
            lcd.setCursor(11, 0);
            lcd.print(String(days[dayofweek - 1]));
            break;
        case (3):
            Clear_screen(0); 
            lcd.setCursor(0, 0);
            lcd.print(" " + zero_days + String(day_t) + "." + zero_m + String(month_t) + "." + String(year_t) + " " + String(mnth[month_t - 1]));
            break;
        }
    }

void drawSensors() {
	  switch (mode_sens) {
        case 0:
            lcd.setCursor(0, 1);
      			lcd.print(String(dispTemp, 0));
      			lcd.write(223);
            lcd.print("C");
      			lcd.setCursor(5, 1);
      			lcd.print(String(dispHum) + "%" + " " + String(dispPres / 100.0F, 0) + "MPa");
            break;        
        case 1:
      			lcd.setCursor(1, 1);
      			lcd.print(String(dispTemp, 1));
      			lcd.write(223);
            lcd.print("C");
      			lcd.setCursor(10, 1);
      			lcd.print(String(dispHum) + "%");
            break;
        case 2:
      			lcd.setCursor(1, 1);
      			lcd.print(String(dispPres / 100.0F, 0) + "MPa");
      			lcd.setCursor(10, 1);
      			lcd.print(String(Height) + "m");
            break; 
		    case 3:
      			lcd.setCursor(0, 1);
      			lcd.print(String(dispTemp, 1));
      			lcd.write(223);
            lcd.print("C");
      			lcd.setCursor(7, 1);
      			lcd.print("Rain:" + String(dispRain) + "%");
            break;	
    }	
	
}

void Clear_screen(byte y) {
    lcd.setCursor(0, y);
    lcd.print("                 ");
}

void predictRain() {
	// тут делаем линейную аппроксимацию для предсказания погоды
    long averPress = 0;
    for (byte i = 0; i < 10; i++) {
    		bme.takeForcedMeasurement();
    		averPress += bme.readPressure();
    		delay(1);
    }
	
    averPress /= 10;
	
    for (byte i = 0; i < 5; i++) {                   	
        pressure_array[i] = pressure_array[i + 1];     	
    }
	
    pressure_array[5] = averPress;                    	// последний элемент массива теперь - новое давление
	
    sumX = 0;
    sumY = 0;
    sumX2 = 0;
    sumXY = 0;
	
    for (int i = 0; i < 6; i++) {                    	// для всех элементов массива
    		sumX += time_array[i];
    		sumY += (long)pressure_array[i];
    		sumX2 += time_array[i] * time_array[i];
    		sumXY += (long)time_array[i] * pressure_array[i];
    }
	
    a = 0;
    a = (long)6 * sumXY;             					// расчёт коэффициента наклона приямой
    a = a - (long)sumX * sumY;
    a = (float)a / (6 * sumX2 - sumX * sumX);
    delta = a * 6;      								// расчёт изменения давления
    dispRain = map(delta, -250, 250, -100, 100);  // пересчитать в проценты	
}

//----------------Обработка нажатия первой кнопки-----------------
// Переключение режимов отображения времени

void Button1Click() {
    boolean Click = digitalRead(Button_1);    
    if (!flag_button_1 && Click) {
        Click = digitalRead(Button_1);
        if (Click) {
            mode_time++;        			
            if (mode_time == 4) mode_time = 0;
      			if (mode_time == 1) {
            	Clear_screen(1);			
      				drawSensors();				
			      } 
            drawClock();	
            clockTick();
            flag_button_1 = 1;         
        }
    }
    if (flag_button_1 && !Click) flag_button_1 = 0;
}

//----------------Обработка нажатия второй кнопки-----------------
// Переключение режимов показания датчика

void Button2Click() {
    boolean Click = digitalRead(Button_2);    
    if (!flag_button_2 && Click) {
        Click = digitalRead(Button_2);
        if (Click) {
            mode_sens++;           
            if (mode_sens == 4) mode_sens = 0;
      			if (mode_time != 0) {
      				Clear_screen(1);
      				drawSensors();	
			      }
            flag_button_2 = 1;         
         }
    }
    if (flag_button_2 && !Click)   flag_button_2 = 0;
}

//---------------Обработка нажатия третей кнопки-----------------
// Включение/выключение подсветки экрана

void Button3Click() {
    boolean Click = digitalRead(Button_3);    
    if (!flag_button_3 && !Click) {
        delay(10);
        Click = digitalRead(Button_3);
        if (!Click) {
            flag = !flag; 
            if (flag) lcd.backlight();
            else lcd.noBacklight();
            flag_button_3 = 1;                  
        }
    }
    if (flag_button_3 && Click) flag_button_3 = 0;      
}
