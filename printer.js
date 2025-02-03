const printer = require('bindings')('printer_node_electron');


async function printDirect(printOptions) {
  const input = {
    ...printOptions,
    printerName: normalizeString(printOptions.printerName)
  }
  return await printer.printDirect(input)
}

async function getStatusPrinter(printOptions) {
  const input = {
    ...printOptions,
    printerName: normalizeString(printOptions.printerName)
  }
  return await printer.getStatusPrinter(input)
}


function normalizeString(str) {
  return String.raw`${str}`
}

module.exports = {
  getPrinters: printer.getPrinters,
  getDefaultPrinter: printer.getDefaultPrinter,
  printDirect,
  getStatusPrinter,
};
