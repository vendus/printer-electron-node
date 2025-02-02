# printer-node-electron

Node.js e Electron bindings para gerenciamento e impressão direta em impressoras. Suporta Windows e Linux (CUPS).

## Características

- Listar todas as impressoras disponíveis
- Obter impressora padrão do sistema
- Obter status detalhado da impressora
- Impressão direta (raw printing)
- Suporte a TypeScript
- API assíncrona (Promises)
- Compatível com Node.js e Electron

## Requisitos

- Node.js >= 18.20.6
- Electron >= 20.0.0
- Windows ou Linux
- Para Windows: Visual Studio Build Tools
- Para Linux: CUPS development headers (`sudo apt-get install libcups2-dev`)

## Instalação

```bash
npm install printer-node-electron
```

Para desenvolvimento:
```bash
git clone https://github.com/Alexssmusica/printer-node-electron.git
cd printer-node-electron
npm install
npm run rebuild
```

## Uso

### JavaScript

```javascript
const printer = require('printer-node-electron');

// Listar todas as impressoras
printer.getPrinters()
  .then(printers => {
    console.log('Impressoras disponíveis:', printers);
  })
  .catch(console.error);

// Obter impressora padrão
printer.getDefaultPrinter()
  .then(defaultPrinter => {
    console.log('Impressora padrão:', defaultPrinter);
  })
  .catch(console.error);

// Verificar status de uma impressora
printer.getStatusPrinter({ printerName: 'Nome da Impressora' })
  .then(status => {
    console.log('Status da impressora:', status);
  })
  .catch(console.error);

// Imprimir diretamente
const printOptions = {
  printerName: 'Nome da Impressora',
  data: 'Texto para impressão',
  dataType: 'RAW'  // opcional, padrão é 'RAW'
};

printer.printDirect(printOptions)
  .then(result => {
    console.log('Resultado:', result);
  })
  .catch(console.error);
```

### TypeScript

```typescript
import printer, { 
  Printer, 
  PrintDirectOptions, 
  GetStatusPrinterOptions 
} from 'printer-node-electron';

async function exemplo() {
  try {
    // Listar impressoras
    const printers: Printer[] = await printer.getPrinters();
    console.log('Impressoras:', printers);

    // Obter impressora padrão
    const defaultPrinter: Printer = await printer.getDefaultPrinter();
    console.log('Impressora padrão:', defaultPrinter);

    // Verificar status
    const statusOptions: GetStatusPrinterOptions = {
      printerName: 'Nome da Impressora'
    };
    const status: Printer = await printer.getStatusPrinter(statusOptions);
    console.log('Status:', status);

    // Imprimir
    const printOptions: PrintDirectOptions = {
      printerName: 'Nome da Impressora',
      data: Buffer.from('Texto para impressão'),
      dataType: 'RAW'
    };
    const result = await printer.printDirect(printOptions);
    console.log('Resultado:', result);
  } catch (error) {
    console.error('Erro:', error);
  }
}
```

## API

### getPrinters(): Promise<Printer[]>
Lista todas as impressoras instaladas no sistema.

Retorna um array de objetos `Printer`:
```typescript
interface Printer {
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
```

### getDefaultPrinter(): Promise<Printer>
Obtém a impressora padrão do sistema.

Retorna um objeto `Printer`.

### getStatusPrinter(options: GetStatusPrinterOptions): Promise<Printer>
Obtém o status detalhado de uma impressora específica.

```typescript
interface GetStatusPrinterOptions {
    printerName: string;
}
```

### printDirect(options: PrintDirectOptions): Promise<string>
Envia dados diretamente para a impressora.

```typescript
interface PrintDirectOptions {
    printerName: string;
    data: string | Buffer;
    dataType?: 'RAW' | 'TEXT' | 'COMMAND' | 'AUTO';
}
```

#### Valores possíveis para status:
- "ready": impressora pronta
- "offline": impressora offline
- "error": erro genérico
- "paper-jam": papel atolado
- "paper-out": sem papel
- "manual-feed": alimentação manual
- "paper-problem": problema com papel
- "busy": ocupada
- "printing": imprimindo
- "output-bin-full": bandeja de saída cheia
- "not-available": não disponível
- "waiting": aguardando
- "processing": processando
- "initializing": inicializando
- "warming-up": aquecendo
- "toner-low": pouco toner
- "no-toner": sem toner
- "page-punt": problema na página
- "user-intervention": necessita intervenção do usuário
- "out-of-memory": sem memória
- "door-open": porta aberta

## Plataformas Suportadas

- Windows (32/64 bits)
- Linux (CUPS)

## Troubleshooting

### Windows
1. Certifique-se de ter o Visual Studio Build Tools instalado
2. Execute `npm run rebuild` após a instalação
3. Verifique as permissões de acesso às impressoras

### Linux
1. Instale as dependências do CUPS: `sudo apt-get install libcups2-dev`
2. Verifique se o serviço CUPS está rodando: `sudo service cups status`
3. Verifique as permissões do usuário: `sudo usermod -a -G lp $USER`

## Desenvolvimento

```bash
# Clonar o repositório
git clone https://github.com/Alexssmusica/printer-node-electron.git

# Instalar dependências
npm install

# Compilar
npm run rebuild

# Executar testes
node teste.js
```

## Licença

ISC

## Contribuindo

1. Fork o projeto
2. Crie sua Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a Branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## Changelog

### 1.0.0
- Implementação inicial
- Suporte a Windows e Linux
- API básica de impressão e gerenciamento de impressoras

## Autor

Alex Santos de Souza - [@Alexssmusica](https://github.com/Alexssmusica)

## Agradecimentos

- Node.js
- node-addon-api
- CUPS
