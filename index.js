const hwinfo = require('./build/Release/hwAddon');
console.log("arranco")
try {
    const sensores = hwinfo.getSensors();
    console.log('Datos de sensores:', sensores);
  } catch (error) {
    console.error('Error al obtener sensores:', error);
  }