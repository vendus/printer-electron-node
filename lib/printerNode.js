"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.printDirect = printDirect;
exports.getStatusPrinter = getStatusPrinter;
exports.getPrinters = getPrinters;
exports.getDefaultPrinter = getDefaultPrinter;
const bindings_1 = __importDefault(require("bindings"));
const printerNode = (0, bindings_1.default)('printer_electron_node');
async function printDirect(printOptions) {
    const input = {
        ...printOptions,
        printerName: normalizeString(printOptions.printerName)
    };
    const printer = await printerNode.printDirect(input);
    return printer;
}
async function getStatusPrinter(printOptions) {
    const input = {
        ...printOptions,
        printerName: normalizeString(printOptions.printerName)
    };
    const printer = await printerNode.getStatusPrinter(input);
    return printer;
}
async function getPrinters() {
    const printers = await printerNode.getPrinters();
    return printers;
}
async function getDefaultPrinter() {
    const printer = await printerNode.getDefaultPrinter();
    return printer;
}
function normalizeString(str) {
    return String.raw `${str}`;
}
