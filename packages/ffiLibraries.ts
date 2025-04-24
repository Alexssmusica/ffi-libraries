type FunctionDefinition = [string, string[]];

interface FunctionDefinitions {
  [key: string]: FunctionDefinition;
}

type FFICallback<T> = (error: any, value: T) => void;

export interface ForeignFunction<TReturn = any, TArgs extends any[] = any[]> {
  (...args: TArgs): TReturn;
  async(...args: [...TArgs, FFICallback<TReturn>]): void;
}

const ffiBindings = require('bindings')('ffi_libraries');

export interface Library {
  /**
   * @param path Path to the dynamic library
   * @param functions Object containing function definitions
   */
  new <T>(path: string, functions: FunctionDefinitions): T;

  /**
   * @param path Path to the dynamic library
   * @param functions Object containing function definitions
   */
  <T>(path: string, functions: FunctionDefinitions): T;
}

/**
 * Creates a new library instance with function definitions
 * @param {string} path Path to the dynamic library
 * @param {Object} functions Object containing function definitions
 * @returns {Object} Object containing the defined functions
 * @example
 * const lib = new Library('user32.dll', {
 *   'MessageBoxA': ['int32', ['pointer', 'string', 'string', 'uint32']],
 *   'GetLastError': ['uint32', []]
 * });
 */
class LibraryImpl {
  constructor(path: string, functions: FunctionDefinitions) {
    if (typeof path !== 'string') {
      throw new TypeError('Library path must be a string');
    }
    if (typeof functions !== 'object' || functions === null) {
      throw new TypeError('Functions definition must be an object');
    }
    const library = new ffiBindings.Library(path, functions);
    Object.assign(this, library);
  }

  /**
   * Closes the library and frees resources
   */
  close(): void {
    if (typeof (this as any).close === 'function') {
      (this as any).close();
    }
  }
}

export const Library: Library = LibraryImpl as any;
