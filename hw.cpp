#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <stdexcept>
#include <cstdint>

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
    void Open() {
        masterSensorNames.clear();
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
            auto sensorElement = reinterpret_cast<_HWiNFO_SENSORS_SENSOR_ELEMENT*>(
                static_cast<uint8_t*>(pMapView) + offsetSensorSection + (dwSensor * sizeSensorElement)
                );

            masterSensorNames.push_back(sensorElement->szSensorNameUser);

            std::cout << "dwSensorID : " << sensorElement->dwSensorID << std::endl;
            std::cout << "dwSensorInst : " << sensorElement->dwSensorInst << std::endl;
            std::cout << "szSensorNameOrig : " << sensorElement->szSensorNameOrig << std::endl;
            std::cout << "szSensorNameUser : " << sensorElement->szSensorNameUser << std::endl;
        }

        // Leer lecturas
        for (uint32_t dwReading = 0; dwReading < numReadingElements; ++dwReading) {
            auto readingElement = reinterpret_cast<_HWiNFO_SENSORS_READING_ELEMENT*>(
                static_cast<uint8_t*>(pMapView) + offsetReadingSection + (dwReading * sizeReadingSection)
                );

            std::cout << "tReading : " << readingElement->tReading << std::endl;
            std::cout << "dwSensorIndex : " << readingElement->dwSensorIndex << " ; Sensor Name: " << masterSensorNames[readingElement->dwSensorIndex] << std::endl;
            std::cout << "dwReadingID : " << readingElement->dwReadingID << std::endl;
            std::cout << "szLabelUser : " << readingElement->szLabelUser << std::endl;
            std::cout << "szUnit : " << readingElement->szUnit << std::endl;
            std::cout << "Value : " << readingElement->Value << std::endl;
        }

        UnmapViewOfFile(pMapView);
        CloseHandle(hMapFile);
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

int main() {
    try {
        HWiNFOWrapper hwinfoWrapper;
        hwinfoWrapper.Open();
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

 

    return 0;
}
