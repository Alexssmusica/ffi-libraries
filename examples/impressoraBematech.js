const { Library } = require('../lib');
const path = require('path');
const _libraryPath = path.join(__dirname, 'mp2064.dll');

// Definições mais simples usando o formato básico do node-ffi
const funcDefs = {
  IniciaPorta: ['int32', ['string']],
  FechaPorta: ['int32', []],
  AcionaGuilhotina: ['int32', ['int32']]
};

console.log('Definições de funções:', funcDefs);

const lib = new Library(_libraryPath, funcDefs);

// Teste cada função com try/catch para ver erros específicos
async function iniciar() {
  try {
    const abreConexaoImpressora = await new Promise((resolve, reject) => {
      lib.IniciaPorta.async('USB', (err, result) => {
        console.log('IniciaPorta result:', result);
        if (err) reject(err);
        else resolve(result);
      });
    });
    console.log('AbreConexaoImpressora result:', abreConexaoImpressora);

    const corteTotal = await new Promise((resolve, reject) => {
      lib.AcionaGuilhotina.async(1, (err, result) => {
        if (err) reject(err);
        else resolve(result);
      });
    });
    console.log('AcionaGuilhotina result:', corteTotal);

    const fecharConexaoImpressora = await new Promise((resolve, reject) => {
      lib.FechaPorta.async((err, result) => {
        if (err) reject(err);
        else resolve(result);
      });
    });
    console.log('FechaConexaoImpressora result:', fecharConexaoImpressora);
  } catch (error) {
    console.error('Error:', error);
    console.error('Stack:', error.stack);
  }
}

iniciar()