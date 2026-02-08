#include <Arduino.h>
#include <Wire.h>

// MPU6050用ライブラリ
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// LovyanGFX v1の日本語フォントを有効にする
#define LGFX_USE_V1_FONT_JP
#define LGFX_AUTODETECT
#include <LovyanGFX.hpp>

// 左側ディスプレイ(0x3C)用の設定
class LGFX_Left : public lgfx::LGFX_Device
{
  lgfx::Panel_SSD1306 _panel_instance;
  lgfx::Bus_I2C       _bus_instance;

public:
  LGFX_Left(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.i2c_port = 0;              // 使用するI2Cポート (0 or 1)
      cfg.freq_write = 400000;       // 送信クロック
      cfg.freq_read  = 400000;       // 受信クロック
      cfg.pin_scl = 22;              // SCLピン
      cfg.pin_sda = 21;              // SDAピン
      cfg.i2c_addr = 0x3C;           // I2Cデバイスアドレス
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.panel_width = 128;
      cfg.panel_height = 64;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2; // ここに 2 を指定するとデフォルトで180度回転します
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

// 右側ディスプレイ(0x3D)用の設定
class LGFX_Right : public lgfx::LGFX_Device
{
  lgfx::Panel_SSD1306 _panel_instance;
  lgfx::Bus_I2C       _bus_instance;

public:
  LGFX_Right(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.i2c_port = 0;              // 使用するI2Cポート (0 or 1)
      cfg.freq_write = 400000;       // 送信クロック
      cfg.freq_read  = 400000;       // 受信クロック
      cfg.pin_scl = 22;              // SCLピン
      cfg.pin_sda = 21;              // SDAピン
      cfg.i2c_addr = 0x3D;           // I2Cデバイスアドレス
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.panel_width = 128;
      cfg.panel_height = 64;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = 2; // ここに 2 を指定するとデフォルトで180度回転します
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

static LGFX_Left lgfx_left;
static LGFX_Right lgfx_right;
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("OLED Test");

  // 左側ディスプレイの初期化
  if (!lgfx_left.init()) {
    Serial.println("Left display initialization failed");
    return;
  }

  // 右側ディスプレイの初期化
  if (!lgfx_right.init()) {
    Serial.println("Right display initialization failed");
    return;
  }

  // 日本語フォントを設定
  lgfx_left.setFont(&fonts::lgfxJapanGothicP_12);
  lgfx_right.setFont(&fonts::lgfxJapanGothicP_12);

  // MPU6050の初期化
  // Wire.begin()はLovyanGFXのinit内ですでに行われていますが、
  // 明示的にアドレス 0x68 を指定して開始します
  if (!mpu.begin(0x68)) {
    Serial.println("Failed to find MPU6050 chip");
    lgfx_left.println("MPU6050が見つかりません");
    while (1) { delay(10); }
  }
  Serial.println("MPU6050 Found!");

  // MPU6050の設定（任意）
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  lgfx_left.clear();
  lgfx_right.clear();

}

void loop() {
  // put your main code here, to run repeatedly:
  // データの取得
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // --- 左側ディスプレイ：加速度表示 ---
  lgfx_left.startWrite();
  lgfx_left.setCursor(0, 0);
  lgfx_left.fillScreen(TFT_BLACK);
  lgfx_left.println("【加速度 m/s^2】");
  lgfx_left.printf("AX: %.2f\n", a.acceleration.x);
  lgfx_left.printf("AY: %.2f\n", a.acceleration.y);
  lgfx_left.printf("AZ: %.2f\n", a.acceleration.z);
  lgfx_left.endWrite();


///*  // --- 右側ディスプレイ：ジャイロ & 温度表示 ---
  lgfx_right.startWrite();
  lgfx_right.setCursor(0, 0);
  lgfx_right.fillScreen(TFT_BLACK);
  lgfx_right.println("【ジャイロ rad/s】");
  lgfx_right.printf("GX: %.2f\n", g.gyro.x);
  lgfx_right.printf("GY: %.2f\n", g.gyro.y);
  lgfx_right.printf("GZ: %.2f\n", g.gyro.z);
  //lgfx_right.println("----------------");
  //lgfx_right.printf("温度: %.1f C\n", temp.temperature);
  lgfx_right.endWrite();
//*/
/*
// --- 右側：水平儀アニメーション ---
  
  // 画面の中心座標
  int16_t centerX = lgfx_left.width() / 2;
  int16_t centerY = lgfx_left.height() / 2;

  // 感度設定（加速度 m/s^2 をピクセル移動量に変換する係数）
  // 縦置きの場合、傾きに応じて Y と Z に重力が分配されるのを利用します
  float sensitivity = 30.0; //重力円の半径 

  // 円の座標を計算 (加速度に応じて中心からオフセット)
  // 設置の向きに合わせて + と - を入れ替えて調整してください
  int16_t ballX = centerX - (int16_t)(sensitivity*a.acceleration.y/9.8);
  int16_t ballY = centerY + (int16_t)(sensitivity*a.acceleration.z/9.8);

  // 画面外に飛び出さないように制限
  ballX = constrain(ballX, 5, 123);
  ballY = constrain(ballY, 5, 59);

  lgfx_right.startWrite();
  lgfx_right.fillScreen(TFT_BLACK); // 画面リセット

  // 1. 中心十字線の描画
  lgfx_right.drawFastHLine(32, centerY, 64, TFT_LIGHTGREY);
  lgfx_right.drawFastVLine(centerX, 0, 64, TFT_LIGHTGREY);

  // 2. 固定の中心ガイド（小さな円）
  lgfx_right.drawCircle(centerX, centerY, sensitivity, TFT_WHITE);

  // 3. 加速度で動く丸（現在の傾き）
  // fillCircle(x, y, 半径, 色)
  lgfx_right.fillCircle(ballX, ballY, 5, TFT_WHITE);

  lgfx_right.endWrite();
  */

  delay(100); // 更新間隔

}