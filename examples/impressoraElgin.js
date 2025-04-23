const { Library } = require('../lib');
const path = require('path');
const _libraryPath = path.join(__dirname, 'E1_Impressora01.dll');

const lib = new Library(_libraryPath, {
  AbreConexaoImpressora: [
    'int32', ['int32', 'string', 'string', 'int32']
  ],
  FechaConexaoImpressora: [
    'int32', []
  ],
  CorteTotal: [
    'int32', ['int32']
  ],
  ImpressaoTexto: [
    'int32', ['string', 'int32', 'int32', 'int32']
  ]
});

console.log(lib);

async function iniciar() {
  try {
    const abreConexaoImpressora = await new Promise((resolve, reject) => {
      const result = lib.AbreConexaoImpressora(0, 'USB', 'i9', 1);
      resolve(result);
    });
    console.log('AbreConexaoImpressora result:', abreConexaoImpressora);

    const corteTotal = await new Promise((resolve, reject) => {
      lib.CorteTotal.async(1, (err, result) => {
        if (err) reject(err);
        else resolve(result);
      });
    });
    console.log('CorteTotal result:', corteTotal);

    const fecharConexaoImpressora = await new Promise((resolve, reject) => {
      lib.FechaConexaoImpressora.async((err, result) => {
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
