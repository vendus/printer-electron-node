/// <reference types="node" />

export interface PrintOptions {
    printerName: string;
    data: string | Buffer;
    dataType?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO' | undefined;
}

export interface Printer {
    name: string;
}

export interface PrinterOptions {
    name: string;
    isDefault: boolean;
    options: {
        location?: string;
        comment?: string;
        driver?: string;
        port?: string;
        [key: string]: string | undefined;
    };
    status: string;
}

export interface PrintDirectOptions {
    printerName: string;
    data: string | Buffer;
    dataType?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO' | undefined;
}

export interface GetStatusPrinterOptions {
    printerName: string;
}

declare module 'printer-node-electron' {
    /**
     * Imprime dados diretamente na impressora
     * @param options Opções de impressão
     * @returns Promise que resolve com mensagem de sucesso
     */
    function printDirect(options: PrintDirectOptions): Promise<string>;

    /**
     * Lista todas as impressoras instaladas
     * @returns Promise que resolve com array de informações das impressoras
     */
    function getPrinters(): Promise<PrinterOptions[]>;

    /**
     * Obtém a impressora padrão do sistema
     * @returns Promise que resolve com informações da impressora padrão
     */
    function getDefaultPrinter(): Promise<PrinterOptions>;

    /**
     * Obtém o status de uma impressora específica
     * @param options Opções com nome da impressora
     * @returns Promise que resolve com informações da impressora
     */
    function getStatusPrinter(options: GetStatusPrinterOptions): Promise<PrinterOptions>;
}

export default 'printer-node-electron';
  