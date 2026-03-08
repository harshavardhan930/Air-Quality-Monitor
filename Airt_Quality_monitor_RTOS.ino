#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <DHT.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

/* Screen resolution */
static const uint16_t screenWidth  = 240;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight);

/* Sensors */
#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define FIRE_SENSOR_PIN 14
#define BUZZER_PIN 33
#define MQ2_SENSOR_PIN 13
#define MQ7PIN 34

/* Dust Sensor */
int measurePin = 12;
int ledPower = 13;

unsigned int samplingTime = 280;
unsigned int deltaTime = 40;
unsigned int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

/* Shared sensor data */
typedef struct
{
  float temperature;
  float humidity;
  float co;
  float dust;
  int fire;
} SensorData;

SensorData data;

/* Mutex */
SemaphoreHandle_t dataMutex;

/* Display flush */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/* Buzzer */
void FirebeepBuzzer(int duration)
{
  digitalWrite(BUZZER_PIN, HIGH);
  vTaskDelay(pdMS_TO_TICKS(duration));
  digitalWrite(BUZZER_PIN, LOW);
}

void LPGbeepBuzzer(int duration)
{
  digitalWrite(BUZZER_PIN, HIGH);
  vTaskDelay(pdMS_TO_TICKS(duration));
  digitalWrite(BUZZER_PIN, LOW);
}

/* ---------------- GUI TASK ---------------- */
void guiTask(void *pvParameters)
{
  while (1)
  {
    lv_timer_handler();

    if (xSemaphoreTake(dataMutex, portMAX_DELAY))
    {
      int temperatureInt = (int)data.temperature;
      int humidityInt = (int)data.humidity;
      int coInt = (int)data.co;

      int aqi = (temperatureInt + humidityInt + coInt) / 3;

      lv_label_set_text_fmt(ui_Label5, "%d", temperatureInt);
      lv_arc_set_value(ui_Arc2, temperatureInt);

      lv_label_set_text_fmt(ui_Label1, "%d", humidityInt);
      lv_arc_set_value(ui_Arc3, humidityInt);

      lv_label_set_text_fmt(ui_Label11, "%d", coInt);

      char dustStr[10];
      dtostrf(data.dust,4,2,dustStr);
      lv_label_set_text(ui_Label2,dustStr);

      lv_label_set_text_fmt(ui_Label4,"%d",aqi);
      lv_arc_set_value(ui_Arc1,aqi);
      lv_slider_set_value(ui_Slider2,aqi,LV_ANIM_OFF);

      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

/* ---------------- SENSOR TASK ---------------- */

void sensorTask(void *pvParameters)
{
  while (1)
  {

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    int mq7Value = analogRead(MQ7PIN);
    float co_ppm = mq7Value * (5.0 / 4096.0) * 50;

    /* Dust sensor */
    digitalWrite(ledPower, LOW);
    delayMicroseconds(samplingTime);

    voMeasured = analogRead(measurePin);

    delayMicroseconds(deltaTime);
    digitalWrite(ledPower, HIGH);

    calcVoltage = voMeasured * (5.0 / 1024);
    dustDensity = 0.17 * calcVoltage - 0.1;

    if (dustDensity < 0)
      dustDensity = 0;

    int fireDetected = digitalRead(FIRE_SENSOR_PIN);

    if (xSemaphoreTake(dataMutex, portMAX_DELAY))
    {
      data.temperature = temperature;
      data.humidity = humidity;
      data.co = co_ppm;
      data.dust = dustDensity;
      data.fire = fireDetected;

      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

/* ---------------- ALARM TASK ---------------- */

void alarmTask(void *pvParameters)
{
  while (1)
  {
    int fireDetected = digitalRead(FIRE_SENSOR_PIN);
    int LPGDetected = digitalRead(MQ2_SENSOR_PIN);

    if (fireDetected == LOW)
    {
      lv_obj_set_style_bg_opa(ui_Panel12,255,LV_PART_MAIN);
      FirebeepBuzzer(200);
    }
    else
    {
      lv_obj_set_style_bg_opa(ui_Panel12,0,LV_PART_MAIN);
    }

    if (LPGDetected == LOW)
    {
      lv_obj_set_style_bg_opa(ui_Panel11,255,LV_PART_MAIN);
      LPGbeepBuzzer(200);
    }
    else
    {
      lv_obj_set_style_bg_opa(ui_Panel11,0,LV_PART_MAIN);
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/* ---------------- BLUETOOTH TASK ---------------- */

void bluetoothTask(void *pvParameters)
{
  while (1)
  {
    if (xSemaphoreTake(dataMutex, portMAX_DELAY))
    {

      SerialBT.print(data.temperature);
      SerialBT.print(",");
      SerialBT.print(data.humidity);
      SerialBT.print(",");
      SerialBT.print(data.co);
      SerialBT.print(",");
      SerialBT.print(data.dust);
      SerialBT.print(",");
      SerialBT.println(data.fire == LOW ? "No" : "Yes");

      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

/* ---------------- SETUP ---------------- */

void setup()
{
  Serial.begin(115200);
  SerialBT.begin("ESP32_BT_Server");

  pinMode(FIRE_SENSOR_PIN,INPUT);
  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(MQ2_SENSOR_PIN,INPUT);
  pinMode(ledPower,OUTPUT);

  dht.begin();

  /* LVGL init */
  lv_init();

  tft.begin();
  tft.setRotation(0);

  uint16_t calData[5] = {300,3266,363,3504,4};
 // tft.setTouch(calData);

  lv_disp_draw_buf_init(&draw_buf,buf,NULL,screenWidth*screenHeight/10);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;

  lv_disp_drv_register(&disp_drv);

  ui_init();

  /* Mutex */
  dataMutex = xSemaphoreCreateMutex();

  /* Tasks */

  xTaskCreatePinnedToCore(guiTask,"GUI",10000,NULL,2,NULL,1);

  xTaskCreatePinnedToCore(sensorTask,"Sensors",4096,NULL,1,NULL,1);

  xTaskCreatePinnedToCore(alarmTask,"Alarm",2048,NULL,1,NULL,1);

  xTaskCreatePinnedToCore(bluetoothTask,"Bluetooth",4096,NULL,1,NULL,0);
}

/* Loop unused because RTOS handles tasks */

void loop()
{
  vTaskDelay(portMAX_DELAY);
}
