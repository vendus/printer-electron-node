{
  "targets": [
    {
      "target_name": "printer_electron_node",
      "sources": [
        "src/main.cpp",
        "src/print.cpp",
        "src/printer_factory.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "defines": [ "NAPI_CPP_EXCEPTIONS" ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "ExceptionHandling": 1
        }
      },
      "binding_name": "printer_electron_node",
      "conditions": [
        ['OS=="win"', {
          "sources": ["src/windows_printer.cpp"],
          "libraries": ["winspool.lib"]
        }],
        ['OS=="linux" or OS=="mac"', {
          "sources": ["src/linux_printer.cpp"],
          "libraries": ["-lcups"],
          "include_dirs": [
            "/usr/include/cups"
          ],
          "cflags": ["-Wall"],
          "xcode_settings": {
            "OTHER_CFLAGS": ["-Wall"],
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
          }
        }]
      ]
    }
  ]
}
