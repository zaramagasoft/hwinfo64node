# hwinfo64node

hwinfo64node read shared memory to json for nodejs in Windows.

## Descripción

Addon for nodejs, it uses node-addon-api. You need configure HWiNFO64 to acces to shared memory.
in npm-->
npm i hwinfonode
if in your main.js--> add
```javascript
const hwinfo = require('hwinfonode'); 
```
in console returns -->Json

## Examples and configures
```javascript
const hwinfo = require('hwinfonode/build/Release/hwAddon');
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
set up your HwInfo
![1](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/1.png)
![2](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/2.png)
![3](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/3.png)
![Hwinfo64support](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/Hwinfo64support.png)
Json received
![json](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/json.png)
![sensorsView](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/sensorsViews.png)
Desired Datas 
![lessDatasInJson](https://github.com/zaramagasoft/hwinfo64node/blob/master/img/lessDatas.png)

### code example index.js


```javascript
const hwinfo = require('hwinfonode/build/Release/hwAddon');
//console.log("arranco")
try {
    const sensores = hwinfo.getSensors();
    console.log('Datos de sensores:', sensores);
  } catch (error) {
    console.error('Error al obtener sensores:', error);
  }
  ```

