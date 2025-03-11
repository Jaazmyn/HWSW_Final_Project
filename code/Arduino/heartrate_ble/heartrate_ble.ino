#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"  // 包含心率检测算法的头文件
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ------------------------- Sensor 参数 -------------------------
// 使用 MAX30105 传感器采集心率数据
MAX30105 particleSensor;

// 用于平滑心率的平均采样参数
const byte RATE_SIZE = 4;   // 平均平滑的采样数
byte rates[RATE_SIZE];       // 存储每次检测到的心率值（BPM）
byte rateSpot = 0;           // 当前数组索引
long lastBeat = 0;           // 上一次检测到心跳的时间
float beatsPerMinute;        // 本次计算出的瞬时心率（BPM）
int beatAvg;                 // 平均心率（BPM）
byte validBeatsCount = 0;    // 有效心跳计数

// ------------------------- BLE 参数 -------------------------
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000; // 每1秒发送一次

// 服务与特征 UUID（请根据需要更换）
#define SERVICE_UUID        "f7cae14c-73e9-4bdb-92fd-c23945b65fe5"
#define CHARACTERISTIC_UUID "7d5ad17e-c928-4485-8338-0f8d6b0a3967"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing MAX30105 & BLE...");

  // 初始化 I2C 总线
  Wire.begin();

  // 初始化传感器
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  // 配置传感器（使用默认设置）
  particleSensor.setup();
  // 设置 LED 亮度，降低红光 LED 亮度可减少光漂移影响
  particleSensor.setPulseAmplitudeRed(0x0A);
  // 如不需要绿色LED，则关闭
  particleSensor.setPulseAmplitudeGreen(0);

  // ------------------------- BLE 初始化 -------------------------
  BLEDevice::init("HeartRateSensor");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE |
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("Waiting for Heart Rate...");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE setup complete. Now broadcasting data...");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // 读取传感器 IR 数据
  long irValue = particleSensor.getIR();

  // 如果 IR 信号较低，说明可能没有检测到手指
  if (irValue < 50000) {
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.println(" No finger detected.");
    validBeatsCount = 0;  // 重置有效心跳计数，等待稳定采样
  } else {
    // 检查是否检测到心跳（使用 checkForBeat 算法）
    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);  // 计算心率 BPM
      
      // 检查心率是否在合理范围内
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;
        if (validBeatsCount < RATE_SIZE) {
          validBeatsCount++;
        }
        // 当收集到足够的有效采样后，计算平均心率
        if (validBeatsCount >= RATE_SIZE) {
          beatAvg = 0;
          for (byte x = 0; x < RATE_SIZE; x++) {
            beatAvg += rates[x];
          }
          beatAvg /= RATE_SIZE;
        }
      }
    }
  }

  // 输出调试信息
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  if (validBeatsCount >= RATE_SIZE) {
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);
  } else {
    Serial.print("Calculating...");
  }
  Serial.println();

  // 每隔 interval 毫秒，通过 BLE 发送最新的心率数据
  if (deviceConnected && (currentMillis - previousMillis >= interval)) {
    previousMillis = currentMillis;
    char dataStr[50];
    if (validBeatsCount >= RATE_SIZE) {
      snprintf(dataStr, sizeof(dataStr), "HR: %d bpm", beatAvg);
    } else {
      snprintf(dataStr, sizeof(dataStr), "Calculating...");
    }
    pCharacteristic->setValue(dataStr);
    pCharacteristic->notify();
    Serial.print("Sent BLE notification: ");
    Serial.println(dataStr);
  }

  // 处理 BLE 连接状态的变化
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);  // 给蓝牙栈一定时间重启广告
    pServer->startAdvertising();
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
  
  delay(10);
}