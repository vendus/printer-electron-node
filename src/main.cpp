#include <napi.h>
#include "print.h"

// Função de inicialização do módulo
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "printDirect"), Napi::Function::New(env, PrintDirect));
    exports.Set(Napi::String::New(env, "getPrinters"), Napi::Function::New(env, GetPrinters));
    exports.Set(Napi::String::New(env, "getDefaultPrinter"), Napi::Function::New(env, GetSystemDefaultPrinter));
    exports.Set(Napi::String::New(env, "getStatusPrinter"), Napi::Function::New(env, GetStatusPrinter));
    return exports;
}

// Define o módulo Node
NODE_API_MODULE(printer_node_electron, Init)
