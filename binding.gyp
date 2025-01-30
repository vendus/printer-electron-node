{
  "targets": [
    {
      "target_name": "printer_node_electron",
      "sources": [
        "src/main.cpp",
        "src/print.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
      "binding_name": "print_lib",
      "libraries": []
    }
  ]
}
