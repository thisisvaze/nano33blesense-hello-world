#include <Arduino_APDS9960.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
#include <Arduino_LSM9DS1.h>
#include <PDM.h>

int16_t minMicrophoneSample;
int16_t maxMicrophoneSample;
int microphoneSamples;
char buff[100];

void PDM_DataReceivedCallback() {
    int16_t val;

    while (PDM.available()) {
        PDM.read(&val, 2);
    
        if (val < minMicrophoneSample) {
            minMicrophoneSample = val;
        }

        if (val > maxMicrophoneSample) {
            maxMicrophoneSample = val;
        }

        microphoneSamples++;
    }

}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
  
    if (!HTS.begin()) {
        Serial.print("HTS init failed");
        while (1) {}
    }
    
    if (!BARO.begin()) {
        Serial.print("BARO init failed");
        while (1) {}
    }
    
    if (!IMU.begin()) {
        Serial.print("IMU init failed");
        while (1) {}
    }
    
    if (!APDS.begin()) {
        Serial.print("APDS init failed");
        while (1) {}
    }
    APDS.setGestureSensitivity(90);

    minMicrophoneSample = INT16_MAX;
    maxMicrophoneSample = INT16_MIN;
    microphoneSamples = 0;

    PDM.onReceive(PDM_DataReceivedCallback);
    if (!PDM.begin(1, 16000)) {
        Serial.print("PDM init failed");
        while (1) {}
    }
}

void loop() {
    float temperature = HTS.readTemperature();
    float humidity = HTS.readHumidity();
    float pressure = BARO.readPressure();

    bool accelAvailable;
    float accelX = -1;
    float accelY = -1;
    float accelZ = -1;
    if ((accelAvailable = IMU.accelerationAvailable())) {
        IMU.readAcceleration(accelX, accelY, accelZ);
    }
    
    bool gyroAvailable;
    float gyroX = -1;
    float gyroY = -1;
    float gyroZ = -1;
    if ((gyroAvailable = IMU.gyroscopeAvailable())) {
        IMU.readGyroscope(gyroX, gyroY, gyroZ);
    }
    
    bool magAvailable;
    float magX = -1;
    float magY = -1;
    float magZ = -1;
    if ((magAvailable = IMU.magneticFieldAvailable())) {
        IMU.readMagneticField(magX, magY, magZ);
    }
    
    bool colorAvailable;
    int colorR = -1;
    int colorG = -1;
    int colorB = -1;
    if ((colorAvailable = APDS.colorAvailable())) {
        APDS.readColor(colorR, colorG, colorB);
    }

    bool proximityAvailable;
    int proximity = -1;
    if ((proximityAvailable = APDS.proximityAvailable())) {
        proximity = APDS.readProximity();
    }

    bool gestureAvailable;
    int gesture;
    if ((gestureAvailable = APDS.gestureAvailable())) {
        gesture = APDS.readGesture();
    }

    NVIC_DisableIRQ(PDM_IRQn);
    bool microphoneAvailabile = minMicrophoneSample != INT16_MAX && maxMicrophoneSample != INT16_MIN;
    int16_t microphoneMinBackup = minMicrophoneSample;
    int16_t microphoneMaxBackup = maxMicrophoneSample;
    int microphoneSamplesBackup = microphoneSamples;
    
    minMicrophoneSample = INT16_MAX;
    maxMicrophoneSample = INT16_MIN;
    microphoneSamples = 0;

    NVIC_EnableIRQ(PDM_IRQn);

    
    sprintf(buff, "%+7.2f°C %+7.2f%% %+7.2f hPa   ", temperature, humidity, pressure);
    Serial.println(buff);
    if (accelAvailable) {
        sprintf(buff, "accel(%+5.2f, %+5.2f, %+5.2f)  ", accelX, accelY, accelZ);
        Serial.println(buff);
    } else {
        Serial.print("                            ");   
    }
    if (gyroAvailable) {
        sprintf(buff, "gyro(%+7.2f, %+7.2f, %+7.2f)  ", gyroX, gyroY, gyroZ);
        Serial.println(buff);
    } else {
        Serial.print("                                 ");   
    }
    if (magAvailable) {
        sprintf(buff, "mag(%+6.2f, %+6.2f, %+6.2f)    ", magX, magY, magZ);
        Serial.println(buff);
    } else {
        Serial.print("                                  ");   
    }
    if (colorAvailable) {
        sprintf(buff, "clr(%3d, %3d, %3d)  ", colorR, colorG, colorB);
        Serial.println(buff);
    } else {
        Serial.print("                    ");   
    }
    if (proximityAvailable) {
        sprintf(buff, "%+5d  ", proximity);
        Serial.print(buff);
    } else {
        Serial.print("       ");
    }

    switch (gesture) {
        case GESTURE_UP:
            Serial.print("UP    ");
            break;
        case GESTURE_DOWN:
            Serial.print("DOWN  ");
            break;
        case GESTURE_LEFT:
            Serial.print("LEFT  ");
            break;
        case GESTURE_RIGHT:
            Serial.print("RIGHT ");
            break;
        default:
            break;
    }
    
    if (microphoneAvailabile) {
        sprintf(buff,"MIC: %d samples between %d and %d\r\n", microphoneSamplesBackup, microphoneMinBackup, microphoneMaxBackup);
        Serial.println(buff);
    } else {
        Serial.print("\r\n");
    }

    delay(1000);
}
