const { getPrinters, getDefaultPrinter, printDirect, getStatusPrinter } = require('./lib/');

// Testando listagem de impressoras
getPrinters().then((printers) => {
  console.log('Impressoras disponíveis:', printers);
}).catch((error) => {
  console.error('Erro ao listar impressoras:', error);
});

// Testando impressora padrão
getDefaultPrinter().then((printer) => {
  console.log('Impressora padrão:', printer);
}).catch((error) => {
  console.error('Erro ao obter impressora padrão:', error);
});

// Testando impressão
const options = {
  printerName: 'HP508140D7C039(HP Laser MFP 131 133 135-138)',
  data: 'Hello, world!',
  dataType: 'RAW'
};

/*
printDirect(options).then((resp) => {
  console.log(resp);
}).catch(console.error);
*/

// Testando status da impressora
getStatusPrinter({ printerName: 'HP508140D7C039(HP Laser MFP 131 133 135-138)' })
  .then((printerInfo) => {
    console.log('Status da impressora:', printerInfo);
  })
  .catch((error) => {
    console.error('Erro ao obter status da impressora:', error);
  });
