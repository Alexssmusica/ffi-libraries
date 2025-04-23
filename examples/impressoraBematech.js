const { Library } = require('../lib');
const path = require('path');
const _libraryPath = path.join(__dirname, 'mp2064.dll');

const funcDefs = {
  IniciaPorta: ['int32', ['string']],
  FechaPorta: ['int32', []],
  AcionaGuilhotina: ['int32', ['int32']]
};

try {
  const lib = new Library(_libraryPath, funcDefs);
  const result = lib.IniciaPorta('COM1');
  console.log(result);
} catch (error) {
  console.error('Error loading library:', error);
}
