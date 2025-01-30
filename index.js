const printLib = require('bindings')('printer_node_electron');

module.exports = {
  getPrinters: printLib.getPrinters,
  getDefaultPrinter: printLib.getDefaultPrinter,
  printDirect: printLib.printDirect,
  getStatusPrinter: printLib.getStatusPrinter,
};
