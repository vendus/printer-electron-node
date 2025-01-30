#ifndef PRINT_H
#define PRINT_H

#include <napi.h>

// Declaração de funções
Napi::Value PrintDirect(const Napi::CallbackInfo& info);
Napi::Value GetPrinters(const Napi::CallbackInfo& info);
Napi::Value GetSystemDefaultPrinter(const Napi::CallbackInfo& info);
Napi::Value GetStatusPrinter(const Napi::CallbackInfo& info);

#endif
