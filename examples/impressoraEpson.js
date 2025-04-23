const { Library } = require('../lib');
const path = require('path');
const _libraryPath = path.join(__dirname, 'InterfaceEpsonNF_x64.dll');

const lib = new Library(_libraryPath, {
  IniciaPorta: [
    'int32', ['string']
  ],
  FechaPorta: [
    'int32', []
  ],
  Le_Status: [
    'int32', []
  ],
  ImprimeTexto: [
    'int32', ['string']
  ],
  AcionaGuilhotina: [
    'int32', ['int32']
  ]
});

console.log(lib);

async function iniciar() {
  try {
    const abreConexaoImpressora = await new Promise((resolve, reject) => {
      const result = lib.IniciaPorta('USB');
      resolve(result)
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
