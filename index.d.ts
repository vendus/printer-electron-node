/// <reference types="node" />

export interface PrintOptions {
    printerName: string;
    data: string | Buffer;
    dataType?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO' | undefined;
}

export interface Printer {
    name: string;
    isDefault: boolean;
    status: string;
    details: {
        location?: string;
        comment?: string;
        driver?: string;
        port?: string;
        [key: string]: string | undefined;
    };
}

export interface PrintDirectOptions {
    printerName: string;
    data: string | Buffer;
    dataType?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO' | undefined;
}

export interface GetStatusPrinterOptions {
    printerName: string;
}

export interface PrintDirectOutput {
    name: string;
    status: 'success' | 'failed';
}

declare module 'printer-electron-node' {
    /**
     * Imprime dados diretamente na impressora
     * @param options Opções de impressão
     * @returns Promise que resolve com mensagem de sucesso
     */
    function printDirect(options: PrintDirectOptions): Promise<PrintDirectOutput>;

    /**
     * Lista todas as impressoras instaladas
     * @returns Promise que resolve com array de informações das impressoras
     */
    function getPrinters(): Promise<Printer[]>;

    /**
     * Obtém a impressora padrão do sistema
     * @returns Promise que resolve com informações da impressora padrão
     */
    function getDefaultPrinter(): Promise<Printer>;

    /**
     * Obtém o status de uma impressora específica
     * @param options Opções com nome da impressora
     * @returns Promise que resolve com informações da impressora
     */
    function getStatusPrinter(options: GetStatusPrinterOptions): Promise<Printer>;
}

export default 'printer-electron-node';
