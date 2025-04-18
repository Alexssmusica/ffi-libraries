const { Library } = require('../lib');
const path = require('path');
const _libraryPath = path.join(__dirname, 'SAT_BEMATECH_x64.dll');

const lib = new Library(_libraryPath, {
  AtivarSAT: ['string', ['int', 'int', 'string', 'string', 'int']],
  ComunicarCertificadoICPBRASIL: ['string', ['int', 'string', 'string']],
  EnviarDadosVenda: ['string', ['int', 'string', 'string']],
  CancelarUltimaVenda: ['string', ['int', 'string', 'string', 'string']],
  ConsultarSAT: ['string', ['int']],
  TesteFimAFim: ['string', ['int', 'string', 'string']],
  ConsultarStatusOperacional: ['string', ['int', 'string']],
  ConsultarNumeroSessao: ['string', ['int', 'string', 'int']],
  ConfigurarInterfaceDeRede: ['string', ['int', 'string', 'string']],
  AssociarAssinatura: ['string', ['int', 'string', 'string', 'string']],
  VersaoLib: ['string', []],
  GeraNumeroSessao: ['int', []],
  AtualizarSoftwareSAT: ['string', ['int', 'string']],
  ExtrairLogs: ['string', ['int', 'string']],
  BloquearSAT: ['string', ['int', 'string']],
  DesbloquearSAT: ['string', ['int', 'string']],
  TrocarCodigoDeAtivacao: ['string', ['int', 'string', 'int', 'string', 'string']]
});

async function iniciar() {
  try {
    const versao = await new Promise((resolve, reject) => {
      lib.VersaoLib.async((err, result) => {
        if (err) reject(err);
        else resolve(result);
      });
    });
    console.log('VersaoLib result:', versao);
    const numeroSessao = await new Promise((resolve, reject) => {
      lib.GeraNumeroSessao.async((err, result) => {
        if (err) reject(err);
        else resolve(result);
      });
    });
    console.log('GeraNumeroSessao result:', numeroSessao);
    const sat = await new Promise((resolve, reject) => {
      lib.ExtrairLogs.async(numeroSessao, '1256584588', (err, result) => {
        if (err) {
          console.error('ConsultarSAT error:', err);
          reject(err);
        } else {
          resolve(result);
        }
      });
    });
    console.log('ExtrairLogs result:', sat);
  } catch (error) {
    console.error('Error:', error);
    console.error('Stack:', error.stack);
  }
}

iniciar()
