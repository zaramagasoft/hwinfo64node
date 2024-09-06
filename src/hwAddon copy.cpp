#include <napi.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

// Aquí incluyes las definiciones de las estructuras y la clase HWiNFOWrapper

const char* HWiNFO_SENSORS_MAP_FILE_NAME2 = "Global\\HWiNFO_SENS_SM2";
const int HWiNFO_SENSORS_STRING_LEN2 = 128;
const int HWiNFO_UNIT_STRING_LEN = 16;

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
    char szLabelOrig[HWiNFO_SENSORS_STRING_LEN2];
    char szLabelUser[HWiNFO_SENSORS_STRING_LEN2];
    char szUnit[HWiNFO_UNIT_STRING_LEN];
    double Value;
    double ValueMin;
    double ValueMax;
    double ValueAvg;
};

struct _HWiNFO_SENSORS_SENSOR_ELEMENT {
    uint32_t dwSensorID;
    uint32_t dwSensorInst;
    char szSensorNameOrig[HWiNFO_SENSORS_STRING_LEN2];
    char szSensorNameUser[HWiNFO_SENSORS_STRING_LEN2];
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
    HWiNFOWrapper() {}

    Napi::Value Open(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        masterSensorNames.clear();

        HANDLE hMapFile = OpenFileMappingA(FILE_MAP_READ, FALSE, HWiNFO_SENSORS_MAP_FILE_NAME2);
        if (!hMapFile) {
            Napi::Error::New(env, "El archivo de memoria compartida no se encontró.").ThrowAsJavaScriptException();
            return env.Null();
        }

        LPVOID pMapView = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
        if (!pMapView) {
            CloseHandle(hMapFile);
            Napi::Error::New(env, "Error al mapear la vista de archivo.").ThrowAsJavaScriptException();
            return env.Null();
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
            auto sensorElement = reinterpret_cast<_HWiNFO_SENSORS_SENSOR_ELEMENT*>(
                static_cast<uint8_t*>(pMapView) + offsetSensorSection + (dwSensor * sizeSensorElement)
            );

            masterSensorNames.push_back(sensorElement->szSensorNameUser);
        }

        // Aquí deberías devolver un array o un objeto JavaScript con los resultados
        Napi::Array sensorArray = Napi::Array::New(env, masterSensorNames.size());
        for (size_t i = 0; i < masterSensorNames.size(); ++i) {
            sensorArray[i] = Napi::String::New(env, masterSensorNames[i]);
        }

        UnmapViewOfFile(pMapView);
        CloseHandle(hMapFile);

        return sensorArray;  // Devolvemos un array de nombres de sensores
    }

private:
    uint32_t numSensors;
    uint32_t numReadingElements;
    std::vector<std::string> masterSensorNames;

    uint32_t offsetSensorSection;
    uint32_t sizeSensorElement;
    uint32_t offsetReadingSection;
    uint32_t sizeReadingSection;
};

// Función para inicializar el addon
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    HWiNFOWrapper hwinfoWrapper;

    exports.Set(Napi::String::New(env, "open"), Napi::Function::New(env, [&hwinfoWrapper](const Napi::CallbackInfo& info) {
        return hwinfoWrapper.Open(info);
    }));

    return exports;
}

NODE_API_MODULE(addon, Init)

