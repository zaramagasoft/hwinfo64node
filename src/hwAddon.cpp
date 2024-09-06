#include <napi.h>
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <stdexcept>
#include <cstdint>
#include <map>

const char* HWiNFO_SENSORS_MAP_FILE_NAME2 = "Global\\HWiNFO_SENS_SM2";

enum SENSOR_READING_TYPE {
    SENSOR_TYPE_NONE = 0,
    SENSOR_TYPE_TEMP,
    SENSOR_TYPE_VOLT,
    SENSOR_TYPE_FAN,
    SENSOR_TYPE_CURRENT,
    SENSOR_TYPE_POWER,
    SENSOR_TYPE_CLOCK,
    SENSOR_TYPE_USAGE,
    SENSOR_TYPE_OTHER
};

#pragma pack(push, 1)
struct _HWiNFO_SENSORS_READING_ELEMENT {
    SENSOR_READING_TYPE tReading;
    uint32_t dwSensorIndex;
    uint32_t dwReadingID;
    char szLabelOrig[128];
    char szLabelUser[128];
    char szUnit[16];
    double Value;
    double ValueMin;
    double ValueMax;
    double ValueAvg;
};

struct _HWiNFO_SENSORS_SENSOR_ELEMENT {
    uint32_t dwSensorID;
    uint32_t dwSensorInst;
    char szSensorNameOrig[128];
    char szSensorNameUser[128];
};

struct _HWiNFO_SENSORS_SHARED_MEM2 {
    uint32_t dwSignature;
    uint32_t dwVersion;
    uint32_t dwRevision;
    int64_t poll_time;
    uint32_t dwOffsetOfSensorSection;
    uint32_t dwSizeOfSensorElement;
    uint32_t dwNumSensorElements;
    uint32_t dwOffsetOfReadingSection;
    uint32_t dwSizeOfReadingElement;
    uint32_t dwNumReadingElements;
};
#pragma pack(pop)

class HWiNFOWrapper {
public:
    Napi::Object GetSensorsData(const Napi::Env& env) {
        Napi::Object result = Napi::Object::New(env);
        Napi::Array sensorsArray = Napi::Array::New(env);

        for (size_t i = 0; i < sensorData.size(); ++i) {
            Napi::Object sensor = Napi::Object::New(env);
            sensor.Set("name", Napi::String::New(env, sensorData[i].name));

            Napi::Array readingsArray = Napi::Array::New(env);
            for (size_t j = 0; j < sensorData[i].readings.size(); ++j) {
                Napi::Object reading = Napi::Object::New(env);
                const auto& r = sensorData[i].readings[j];
                reading.Set("type", Napi::Number::New(env, r.type));
                reading.Set("label", Napi::String::New(env, r.label));
                reading.Set("unit", Napi::String::New(env, r.unit));
                reading.Set("value", Napi::Number::New(env, r.value));
                readingsArray.Set(j, reading);
            }

            sensor.Set("readings", readingsArray);
            sensorsArray.Set(i, sensor);
        }

        result.Set("sensores", sensorsArray);
        return result;
    }

    void Open() {
        sensorData.clear();
        HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, HWiNFO_SENSORS_MAP_FILE_NAME2);
        if (!hMapFile) {
            throw std::runtime_error("El archivo de memoria compartida no se encontró.");
        }

        LPVOID pMapView = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
        if (!pMapView) {
            CloseHandle(hMapFile);
            throw std::runtime_error("Error al mapear la vista de archivo.");
        }

        _HWiNFO_SENSORS_SHARED_MEM2* pSharedMemory = static_cast<_HWiNFO_SENSORS_SHARED_MEM2*>(pMapView);

        numSensors = pSharedMemory->dwNumSensorElements;
        numReadingElements = pSharedMemory->dwNumReadingElements;
        offsetSensorSection = pSharedMemory->dwOffsetOfSensorSection;
        sizeSensorElement = pSharedMemory->dwSizeOfSensorElement;
        offsetReadingSection = pSharedMemory->dwOffsetOfReadingSection;
        sizeReadingSection = pSharedMemory->dwSizeOfReadingElement;

        // Leer sensores
        for (uint32_t dwSensor = 0; dwSensor < numSensors; ++dwSensor) {
            auto baseAddress = static_cast<uint8_t*>(pMapView);
            auto offset = offsetSensorSection + (dwSensor * sizeSensorElement);

            if (baseAddress == nullptr || offset < 0) {
                std::cerr << "Error: Dirección de memoria inválida para el sensor" << std::endl;
                continue; // Saltar este sensor si hay problemas de punteros
            }

            auto sensorElement = reinterpret_cast<_HWiNFO_SENSORS_SENSOR_ELEMENT*>(baseAddress + offset);

            if (sensorElement == nullptr) {
                std::cerr << "Error: sensorElement no inicializado correctamente" << std::endl;
                continue;
            }

            SensorData sd;
            sd.name = sensorElement->szSensorNameUser;
            sensorData.push_back(sd);
        }

        // Leer lecturas
        for (uint32_t dwReading = 0; dwReading < numReadingElements; ++dwReading) {
            auto baseAddress = static_cast<uint8_t*>(pMapView);
            auto offset = offsetReadingSection + (dwReading * sizeReadingSection);

            if (baseAddress == nullptr || offset < 0) {
                std::cerr << "Error: Dirección de memoria inválida para la lectura" << std::endl;
                continue; // Saltar esta lectura si hay problemas de punteros
            }

            auto readingElement = reinterpret_cast<_HWiNFO_SENSORS_READING_ELEMENT*>(baseAddress + offset);

            if (readingElement == nullptr) {
                std::cerr << "Error: readingElement no inicializado correctamente" << std::endl;
                continue;
            }

            ReadingData rd;
            rd.type = readingElement->tReading;
            rd.label = readingElement->szLabelUser;
            rd.unit = readingElement->szUnit;
            rd.value = readingElement->Value;

            if (readingElement->dwSensorIndex < sensorData.size()) {
                sensorData[readingElement->dwSensorIndex].readings.push_back(rd);
            }
        }

        UnmapViewOfFile(pMapView);
        CloseHandle(hMapFile);
    }

private:
    uint32_t numSensors;
    uint32_t numReadingElements;

    uint32_t offsetSensorSection;
    uint32_t sizeSensorElement;
    uint32_t offsetReadingSection;
    uint32_t sizeReadingSection;

    struct ReadingData {
        SENSOR_READING_TYPE type;
        std::string label;
        std::string unit;
        double value;
    };

    struct SensorData {
        std::string name;
        std::vector<ReadingData> readings;
    };

    std::vector<SensorData> sensorData;
};

// Función que expone el método de obtener los sensores a Node.js
Napi::Object GetSensors(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    HWiNFOWrapper hwinfoWrapper;
    hwinfoWrapper.Open();

    // Devuelve los datos en formato JSON a Node.js
    return hwinfoWrapper.GetSensorsData(env);
}

// Método que expone el módulo
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("getSensors", Napi::Function::New(env, GetSensors));
    return exports;
}

NODE_API_MODULE(hwinfo, Init)


