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
      "conditions": [
        ['OS=="win"', {
          "sources": ["src/windows_printer.cpp"],
          "libraries": ["winspool.lib"],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            }
          }
        }],
        ['OS=="mac"', {
          "sources": ["src/mac_printer.cpp"],
          "libraries": ["-lcups"],
          "include_dirs": [
            "/usr/include/cups"
          ],
          "xcode_settings": {
            "OTHER_CFLAGS": ["-Wall"],
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.15",
            "OTHER_LDFLAGS": [
              "-framework", "CoreFoundation",
              "-framework", "CorePrinting"
            ],
            "GCC_ENABLE_CPP_RTTI": "YES",
            "CLANG_CXX_LANGUAGE_STANDARD": "c++17"
          }
        }],
        ['OS=="linux"', {
          "sources": ["src/linux_printer.cpp"],
          "libraries": ["-lcups"],
          "include_dirs": [
            "/usr/include/cups"
          ],
          "cflags": [
            "-Wall",
            "-fexceptions"
          ],
          "cflags_cc": [
            "-fexceptions"
          ]
        }]
      ]
    }
  ]
}
