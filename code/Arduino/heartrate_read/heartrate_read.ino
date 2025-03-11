#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4;   // 用于平均平滑的采样数量
byte rates[RATE_SIZE];       // 存储有效心率数据的数组
byte rateSpot = 0;           // 当前数组索引
long lastBeat = 0;           // 上一次检测到心跳的时间

float beatsPerMinute;
int beatAvg;
byte validBeatsCount = 0;    // 有效心跳计数

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  // 初始化传感器
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); // 使用默认设置配置传感器
  particleSensor.setPulseAmplitudeRed(0x0A); // 开启红光 LED，较低亮度
  particleSensor.setPulseAmplitudeGreen(0);  // 关闭绿光 LED
}

void loop()
{
  long irValue = particleSensor.getIR();

  // 如果IR信号太弱，可能没检测到手指，不进行心率计算
  if (irValue < 50000) {
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.println(" No finger?");
    // 重置有效心跳计数，防止错误计算
    validBeatsCount = 0;
    delay(100);
    return;
  }

  if (checkForBeat(irValue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute;  // 存储本次心率
      rateSpot %= RATE_SIZE;                     // 环形数组索引

      // 累加有效心跳计数（初期有效心跳数不足时逐步增加）
      if (validBeatsCount < RATE_SIZE) {
        validBeatsCount++;
      }

      // 当有效心跳数量足够时，计算平均心率
      if (validBeatsCount >= RATE_SIZE) {
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++) {
          beatAvg += rates[x];
        }
        beatAvg /= RATE_SIZE;
      }
    }
  }

  // 只有在有效心跳数达到阈值后才显示心率数据
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
  delay(10);
}