const printLib = require('bindings')('printer_electron_node');

module.exports = {
  getPrinters: printLib.getPrinters,
  getDefaultPrinter: printLib.getDefaultPrinter,
  printDirect: printLib.printDirect,
  getStatusPrinter: printLib.getStatusPrinter,
};
