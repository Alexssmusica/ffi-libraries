{
  'targets': [{
    'target_name': 'ffi_libraries',
    'cflags!': [ '-fno-exceptions' ],
    'cflags_cc!': [ '-fno-exceptions' ],
    'sources': [ 
      'src/ffi_loader.cc',
      'src/type_converter.cc',
      'src/native_function_caller.cc',
      'src/library_wrapper.cc',
      'src/library_platform_win32.cc'
    ],
    'include_dirs': [
      "<!@(node -p \"require('node-addon-api').include\")",
      "src"
    ],
    'dependencies': [
      "<!(node -p \"require('node-addon-api').gyp\")"
    ],
    'conditions': [
      ['OS=="win"', {
        'defines': [ 
          'WINDOWS',
          '_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS'
        ],
        'msvs_settings': {
          'VCCLCompilerTool': { 
            'ExceptionHandling': 1
          }
        }
      }]
    ]
  }]
}