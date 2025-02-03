const printerNode = require('bindings')('printer_electron_node');

async function printDirect(printOptions) {
  const input = {
    ...printOptions,
    printerName: normalizeString(printOptions.printerName)
  }
  return await printerNode.printDirect(input)
}

async function getStatusPrinter(printOptions) {
  const input = {
    ...printOptions,
    printerName: normalizeString(printOptions.printerName)
  }
  return await printerNode.getStatusPrinter(input)
}


function normalizeString(str) {
  return String.raw`${str}`
}

module.exports = {
  getPrinters: printerNode.getPrinters,
  getDefaultPrinter: printerNode.getDefaultPrinter,
  printDirect,
  getStatusPrinter,
};
