#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <DHT.h>
#include <BluetoothSerial.h>



BluetoothSerial SerialBT; 


/*Don't forget to set Sketchbook location in File/Preferences to the path of your UI project (the parent folder of this INO file)*/

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

#define DHTPIN 27       // Pin for DHT11 sensor
#define DHTTYPE DHT11   // Define DHT type
DHT dht(DHTPIN, DHTTYPE);

#define FIRE_SENSOR_PIN 14 // Pin connected to the IR fire sensor
#define BUZZER_PIN 33      // Pin connected to the buzzer

#define MQ2_SENSOR_PIN 13 // Pin connected to the IR fire sensor

#define MQ7PIN 34       // Analog pin for MQ7 sensor

//---------------------------------------------------------------------------------
int measurePin = 12;
int ledPower = 13;

unsigned int samplingTime = 280;
unsigned int deltaTime = 40;
unsigned int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
//---------------------------------------------------------------------------------

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX = 0, touchY = 0;

    bool touched = false;//tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}

void handleTouchInput() {
  uint16_t x, y;
  if (tft.getTouch(&x, &y)) {
    // Example Case 1: Top-left quadrant (adjust coordinates based on your screen layout)
    if (x > 0 && x < 120 && y > 0 && y < 160) {
      _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_MOVE_TOP, 1000, 0, &ui_Screen1_screen_init);
    } 
    // Example Case 2: Top-right quadrant
    else if (x >= 120 && x < 240 && y > 0 && y < 160) {
      _ui_screen_change(&ui_Screen2, LV_SCR_LOAD_ANIM_MOVE_LEFT, 1000, 0, &ui_Screen2_screen_init);
    } 
    // Example Case 3: Bottom-left quadrant
    else if (x > 0 && x < 120 && y >= 160 && y < 320) {
      _ui_screen_change(&ui_Screen3, LV_SCR_LOAD_ANIM_MOVE_BOTTOM, 1000, 0, &ui_Screen3_screen_init);
    } 
    // Example Case 4: Bottom-right quadrant
    else if (x >= 120 && x < 240 && y >= 160 && y < 320) {
      _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 1000, 0, &ui_Screen1_screen_init);
    }
  }
}



void setup()
{

  SerialBT.begin("ESP32_BT_Server"); // Set a name for Bluetooth server


    Serial.begin( 115200 ); /* prepare for possible serial debug */
  uint16_t calData[5] = { 300, 3266, 363, 3504, 4 };
  tft.setTouch(calData);


     pinMode(FIRE_SENSOR_PIN, INPUT); // Set fire sensor pin as input
    pinMode(BUZZER_PIN, OUTPUT);     // Set buzzer pin as output
    pinMode(ledPower, OUTPUT);

    pinMode(MQ2_SENSOR_PIN, INPUT); // Set fire sensor pin as input

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println( LVGL_Arduino );
    Serial.println( "I am LVGL_Arduino" );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    tft.begin();          /* TFT init */
    tft.setRotation( 0 ); /* Landscape orientation, flipped */

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

    ui_init();

    dht.begin();

    Serial.println( "Setup done" );
      // Initialize the display
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(10, 10);
  tft.println("Air Monitoring System");
  delay(2000);
}

char dustDensityStr[10];  // Char array to store dust density string

void loop() {
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
    handleTouchInput();

    //------------------------------------------------------------------------------------------------------------
    // Read dust density from the sensor

    // Turn off the LED to start measuring
    digitalWrite(ledPower, LOW);
    delayMicroseconds(samplingTime);

    // Read the analog value from the sensor
    voMeasured = analogRead(measurePin);

    // Delay for the required amount of time
    delayMicroseconds(deltaTime);

    // Turn on the LED after the reading
    digitalWrite(ledPower, HIGH);
    delayMicroseconds(sleepTime);

    // Convert the measured value to voltage
    calcVoltage = voMeasured * (5.0 / 1024);

    // Calculate the dust density (PM2.5)
    dustDensity = 0.17 * calcVoltage - 0.1;

    // Ensure dust density is not negative
    if (dustDensity < 0) {
        dustDensity = 0.00;
    }

    // Convert dust density to string for display
    char dustDensityStr[10];
    dtostrf(dustDensity, 4, 2, dustDensityStr);  // Convert float to string with 2 decimal places

    // Update the label on the UI with the dust density value
    lv_label_set_text(ui_Label2, dustDensityStr);

    //------------------------------------------------------------------------------------------------------------

    // Read temperature and humidity from DHT11
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    


    // Check if data is valid
    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    int temperatureInt = (int)(temperature + 0.5);
    int humidityInt = (int)(humidity + 0.5);

    // Update temperature UI components
    lv_label_set_text_fmt(ui_Label5, "%d", temperatureInt);
    lv_arc_set_value(ui_Arc2, temperatureInt);

    // Update humidity UI components
    lv_label_set_text_fmt(ui_Label1, "%d", humidityInt);
    lv_arc_set_value(ui_Arc3, humidityInt);

    // Read CO level from MQ7
    int mq7Value = analogRead(MQ7PIN);
    float co_ppm = mq7Value * (5.0 / 4096.0)*50;
    int coInt = (int)(co_ppm);
    lv_label_set_text_fmt(ui_Label11, "%d", coInt);

    // Calculate HCHO level from MQ7 reading
    float hcho_ppm = mq7Value * 0.02;
    int hchoInt = (int)(hcho_ppm + 0.5);
    lv_label_set_text_fmt(ui_Label7, "%d", hchoInt);

    // Calculate AQI
    int aqi = (humidityInt + temperatureInt + coInt) / 3;
    lv_label_set_text_fmt(ui_Label4, "%d", aqi);
    lv_arc_set_value(ui_Arc1, aqi);
    lv_slider_set_value(ui_Slider2, aqi, LV_ANIM_OFF);

    // LPG detection panel
    lv_obj_set_style_bg_opa(ui_Panel11, 150, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Fire detection
    int fireDetected = digitalRead(FIRE_SENSOR_PIN);
    if (fireDetected == LOW) { // Assuming LOW indicates fire detected
        Serial.println("Fire detected!");
        lv_obj_set_style_bg_opa(ui_Panel12, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        FirebeepBuzzer(200, 100, 5); // 5 beeps of 200ms each, with 100ms interval
      }
    else {
        digitalWrite(BUZZER_PIN, LOW); // Deactivate buzzer if no fire
        lv_obj_set_style_bg_opa(ui_Panel12, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // Reset opacity
    }
    //-------------------------------------------------------------lpg sensor
    int LPGDetected = digitalRead(MQ2_SENSOR_PIN);
    if (LPGDetected == LOW) { // Assuming LOW indicates fire detected
        Serial.println("LPG detected!");
        lv_obj_set_style_bg_opa(ui_Panel11, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        LPGbeepBuzzer(200, 100, 5); // 5 beeps of 200ms each, with 100ms interval
      }
    else {
        digitalWrite(BUZZER_PIN, LOW); // Deactivate buzzer if no fire
        lv_obj_set_style_bg_opa(ui_Panel11, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // Reset opacity
    }

    Serial.printf("Temp: %d C, Humidity: %d %%, CO: %d ppm, HCHO: %d ppm, AQI: %d\n",
                  temperatureInt, humidityInt, coInt, hchoInt, aqi);


  SerialBT.print(temperature);
  SerialBT.print(",");
  SerialBT.print(humidity);
  SerialBT.print(",");
  SerialBT.print(co_ppm);
  SerialBT.print(",");
  SerialBT.print(dustDensity);
  SerialBT.print(",");
  SerialBT.println(fireDetected == LOW ? "No" : "Yes");

}

void FirebeepBuzzer(int beepDuration, int beepInterval, int numberOfBeeps) {    
    for (int i = 0; i < numberOfBeeps; i++) {
      lv_obj_set_style_bg_opa(ui_Panel12, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
        delay(beepDuration);           // Wait for the beep duration
        lv_obj_set_style_bg_opa(ui_Panel12, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
        delay(beepInterval);           // Wait for the interval between beeps
        lv_obj_set_style_bg_opa(ui_Panel12, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

void LPGbeepBuzzer(int beepDuration, int beepInterval, int numberOfBeeps) {
    for (int i = 0; i < numberOfBeeps; i++) {
      lv_obj_set_style_bg_opa(ui_Panel11, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
        delay(beepDuration);           // Wait for the beep duration
        lv_obj_set_style_bg_opa(ui_Panel11, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
        delay(beepInterval);           // Wait for the interval between beeps
        lv_obj_set_style_bg_opa(ui_Panel11, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}