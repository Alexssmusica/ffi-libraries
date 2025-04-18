# FFI Libraries

[English](#ffi-libraries) | [Português](#ffi-libraries-português)

A powerful Node.js library for loading and calling functions from dynamic libraries (DLLs, SOs, DYLIBs) with support for both Node.js and Electron environments.

## Features

- Load dynamic libraries (DLL, SO, DYLIB) at runtime
- Call functions from loaded libraries with automatic type conversion
- Support for multiple Node.js and Electron versions
- Cross-platform compatibility (Windows, Linux, macOS)
- Pre-built binaries for easy installation
- TypeScript support

## Requirements

- Node.js >= 18.0.0
- Compatible with Electron versions 20.0.0 through 34.0.0

## Development Prerequisites

To develop or build from source, you need:

- Visual Studio Build Tools 2019 or later
  - Windows 10 SDK
  - MSVC v142 x64/x86 build tools
  - C++ development workload
- Python 3.x (required by node-gyp)

### Installing Visual Studio Build Tools

1. Download Visual Studio Build Tools from [Visual Studio Downloads](https://visualstudio.microsoft.com/downloads/)
2. Run the installer
3. Select the following components:
   - MSVC v142 x64/x86 build tools
   - Windows 10 SDK
   - C++ development workload
4. Complete the installation

## Installation

```bash
npm install ffi-libraries
```

## Supported Architectures

- x64 (64-bit)
- ia32 (32-bit)

## Supported Platforms

- Windows

## Usage Examples

### TypeScript Example

```typescript
import { Library } from 'ffi-libraries';
import path from 'path';

// Define the library interface
interface CustomLibrary {
  // Simple function that returns a string
  getMessage: () => string;
  // Function with parameters
  add: (a: number, b: number) => number;
  // Function that receives a buffer
  processBuffer: (data: Buffer) => void;
  // Async function that returns a promise
  doAsyncTask: (value: string) => Promise<string>;
}

// Load the library
const libraryPath = path.join(__dirname, 'library.dll');
const lib = new Library<CustomLibrary>(libraryPath, {
  getMessage: ['string', []],
  add: ['int', ['int', 'int']],
  processBuffer: ['void', ['pointer']],
  doAsyncTask: ['string', ['string']]
});

async function exemplo() {
  try {
    // Simple function call
    const message = await lib.getMessage();
    console.log('Message:', message);

    // Function with parameters
    const sum = await lib.add(5, 3);
    console.log('Sum:', sum);

    // Working with buffers
    const buffer = Buffer.from('Hello World');
    await lib.processBuffer(buffer);

    // Async operation
    const result = await lib.doAsyncTask('test');
    console.log('Async Result:', result);
  } catch (error) {
    console.error('Error:', error);
  }
}

exemplo();
```

### JavaScript Example

```javascript
const { Library } = require('ffi-libraries');
const path = require('path');

// Load the library
const libraryPath = path.join(__dirname, 'library.dll');
const lib = new Library(libraryPath, {
  getMessage: ['string', []],
  add: ['int', ['int', 'int']],
  processBuffer: ['void', ['pointer']],
  doAsyncTask: ['string', ['string']]
});

async function exemplo() {
  try {
    // Simple function call using callback style
    const message = await new Promise((resolve, reject) => {
      lib.getMessage.async((err, result) => {
        if (err) reject(err);
        else resolve(result);
      });
    });
    console.log('Message:', message);
    // Function with parameters
    const sum = await new Promise((resolve, reject) => {
      lib.add.async(5, 3, (err, result) => {
        if (err) reject(err);
        else resolve(result);
      });
    });
    console.log('Sum:', sum);
    // Working with buffers
    const buffer = Buffer.from('Hello World');
    await new Promise((resolve, reject) => {
      lib.processBuffer.async(buffer, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
  } catch (error) {
    console.error('Error:', error);
    console.error('Stack:', error.stack);
  }
}

exemplo();
```

## Building from Source

If you need to build the module from source:

```bash
npm run rebuild
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Author

**Alex Santos de Souza**
- GitHub: [@Alexssmusica](https://github.com/Alexssmusica)
- Email: alexssmusica@gmail.com

## Project Status

Active development - Accepting contributions

## Acknowledgments

- Built with [node-addon-api](https://github.com/nodejs/node-addon-api)
- Uses [prebuild](https://github.com/prebuild/prebuild) for binary management

## Support

For support, issues, or feature requests, please use the [GitHub issues page](https://github.com/Alexssmusica/ffi-libraries/issues).

---

# FFI Libraries (Português)

Uma poderosa biblioteca Node.js para carregar e chamar funções de bibliotecas dinâmicas (DLLs, SOs, DYLIBs) com suporte para ambientes Node.js e Electron.

## Características

- Carregamento de bibliotecas dinâmicas DLL em tempo de execução
- Chamada de funções de bibliotecas carregadas com conversão automática de tipos
- Suporte para múltiplas versões do Node.js e Electron
- Compatibilidade apenas Windows (outros OS em implementações futuras)
- Binários pré-compilados para instalação fácil
- Suporte a TypeScript

## Requisitos

- Node.js >= 18.0.0
- Compatível com versões do Electron de 20.0.0 até 34.0.0

## Pré-requisitos para Desenvolvimento

Para desenvolver ou compilar a partir do código fonte, você precisa:

- Visual Studio Build Tools 2019 ou superior
  - Windows 10 SDK
  - MSVC v142 ferramentas de build x64/x86
  - Carga de trabalho de desenvolvimento C++
- Python 3.x (requerido pelo node-gyp)

### Instalando o Visual Studio Build Tools

1. Baixe o Visual Studio Build Tools em [Visual Studio Downloads](https://visualstudio.microsoft.com/downloads/)
2. Execute o instalador
3. Selecione os seguintes componentes:
   - MSVC v142 ferramentas de build x64/x86
   - Windows 10 SDK
   - Carga de trabalho de desenvolvimento C++
4. Complete a instalação

## Instalação

```bash
npm install ffi-libraries
```

## Arquiteturas Suportadas

- x64 (64 bits)
- ia32 (32 bits)

## Plataformas Suportadas

- Windows
- Linux
- macOS

## Exemplos de Uso

### Exemplo em TypeScript

```typescript
import { Library } from 'ffi-libraries';
import path from 'path';

// Definindo a interface da biblioteca
interface BibliotecaPersonalizada {
  // Função simples que retorna uma string
  obterMensagem: () => string;
  // Função com parâmetros
  somar: (a: number, b: number) => number;
  // Função que recebe um buffer
  processarBuffer: (dados: Buffer) => void;
  // Função assíncrona que retorna uma promise
  executarTarefaAsync: (valor: string) => Promise<string>;
}

// Carregando a biblioteca
const caminhoBiblioteca = path.join(__dirname, 'biblioteca.dll');
const lib = new Library<BibliotecaPersonalizada>(caminhoBiblioteca, {
  obterMensagem: ['string', []],
  somar: ['int', ['int', 'int']],
  processarBuffer: ['void', ['pointer']],
  executarTarefaAsync: ['string', ['string']]
});

async function exemplo() {
  try {
    // Chamada de função simples
    const mensagem = await lib.obterMensagem();
    console.log('Mensagem:', mensagem);

    // Função com parâmetros
    const soma = await lib.somar(5, 3);
    console.log('Soma:', soma);

    // Trabalhando com buffers
    const buffer = Buffer.from('Olá Mundo');
    await lib.processarBuffer(buffer);

    // Operação assíncrona
    const resultado = await lib.executarTarefaAsync('teste');
    console.log('Resultado Async:', resultado);
  } catch (erro) {
    console.error('Erro:', erro);
  }
}

exemplo();
```

### Exemplo em JavaScript

```javascript
const { Library } = require('ffi-libraries');
const path = require('path');

// Carregando a biblioteca
const caminhoBiblioteca = path.join(__dirname, 'biblioteca.dll');
const lib = new Library(caminhoBiblioteca, {
  obterMensagem: ['string', []],
  somar: ['int', ['int', 'int']],
  processarBuffer: ['void', ['pointer']],
  executarTarefaAsync: ['string', ['string']]
});

async function exemplo() {
    // Chamada de função simples usando estilo callback
    const mensagem = await new Promise((resolve, reject) => {
      lib.obterMensagem.async((err, resultado) => {
        if (err) reject(err);
        else resolve(resultado);
      });
    });
    console.log('Mensagem:', mensagem);

    // Função com parâmetros
    const soma = await new Promise((resolve, reject) => {
      lib.somar.async(5, 3, (err, resultado) => {
        if (err) reject(err);
        else resolve(resultado);
      });
    });
    console.log('Soma:', soma);

    // Trabalhando com buffers
    const buffer = Buffer.from('Olá Mundo');
    await new Promise((resolve, reject) => {
      lib.processarBuffer.async(buffer, (err) => {
        if (err) reject(err);
        else resolve();
      });
    });
}

exemplo();
```

## Tipos de Dados Suportados

- **Números**: int8, uint8, int16, uint16, int32, uint32, int64, uint64, float, double
- **Strings**: char*, wchar_t*, UTF-8, UTF-16
- **Buffers**: Buffer, ArrayBuffer, TypedArrays
- **Ponteiros**: void*, handles
- **Estruturas**: structs, unions
- **Callbacks**: funções de callback

## Compilando a partir do Código Fonte

Se você precisar compilar o módulo a partir do código fonte:

```bash
npm run rebuild
```

## Autor

**Alex Santos de Souza**
- GitHub: [@Alexssmusica](https://github.com/Alexssmusica)
- Email: alexssmusica@gmail.com

## Status do Projeto

Em desenvolvimento ativo - Aceitando contribuições

## Suporte

Para suporte, problemas ou solicitações de recursos, por favor utilize a [página de issues do GitHub](https://github.com/Alexssmusica/ffi-libraries/issues). 