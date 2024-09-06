# hwinfo64node

hwinfo64node read shared memory to json for nodejs in Windows.

## Descripción

Addon for nodejs, it uses node-addon-api. You need configure HWiNFO64 to acces to shared memory.

## Examples and configures
```javascript
const hwinfo = require('./build/Release/hwAddon');
console.log("arranco")
setInterval(() => {
    try {
        const sensores = hwinfo.getSensors();
        console.log('Datos de sensores:', sensores);
      } catch (error) {
        console.error('Error al obtener sensores:', error);
      }
}, 2000);
```
![1](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/1.png)
![2](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/2.png)
![3](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/3.png)
![Hwinfo64support](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/Hwinfo64support.png)
![json](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/json.png)
![sensorsView](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/sensorsViews.png)


### code example index.js


```javascript
const addon = require('aida64node/build/Release/addon');

// Leer el buffer compartido
const buffer = addon.readBuffer();
if (buffer) {
  console.log("Buffer leído:", buffer);

  // Parsear los datos del sensor
  const parsedData = addon.parseSensorData(buffer);
  console.log("Datos parseados:", parsedData);
} else {
  console.error("No se pudo leer el buffer compartido.");
}

```const hwinfo = require('./build/Release/hwAddon');
console.log("arranco")
try {
    const sensores = hwinfo.getSensors();
    console.log('Datos de sensores:', sensores);
  } catch (error) {
    console.error('Error al obtener sensores:', error);
  }
  ```

