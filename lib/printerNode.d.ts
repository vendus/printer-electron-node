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
export declare function printDirect(printOptions: PrintOptions): Promise<PrintDirectOutput>;
export declare function getStatusPrinter(printOptions: GetStatusPrinterOptions): Promise<Printer>;
export declare function getPrinters(): Promise<Printer[]>;
export declare function getDefaultPrinter(): Promise<Printer>;
