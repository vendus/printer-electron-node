#include <napi.h>
#include <vector>
#include <string>
#include <map>

#ifdef _WIN32
  #include <windows.h>
  #include <winspool.h>
#else
  #include <cups/cups.h>
  #include <cups/ppd.h>
  #include <cups/http.h>
#endif

#include "print.h"

struct PrinterInfo {
    std::string name;
    bool isDefault;
    std::map<std::string, std::string> options;
    std::string status;
};

#ifdef _WIN32
std::string GetPrinterStatus(DWORD status) {
    if (status & PRINTER_STATUS_OFFLINE) return "offline";
    if (status & PRINTER_STATUS_ERROR) return "error";
    if (status & PRINTER_STATUS_PAPER_JAM) return "paper-jam";
    if (status & PRINTER_STATUS_PAPER_OUT) return "paper-out";
    if (status & PRINTER_STATUS_MANUAL_FEED) return "manual-feed";
    if (status & PRINTER_STATUS_PAPER_PROBLEM) return "paper-problem";
    if (status & PRINTER_STATUS_BUSY) return "busy";
    if (status & PRINTER_STATUS_PRINTING) return "printing";
    if (status & PRINTER_STATUS_OUTPUT_BIN_FULL) return "output-bin-full";
    if (status & PRINTER_STATUS_NOT_AVAILABLE) return "not-available";
    if (status & PRINTER_STATUS_WAITING) return "waiting";
    if (status & PRINTER_STATUS_PROCESSING) return "processing";
    if (status & PRINTER_STATUS_INITIALIZING) return "initializing";
    if (status & PRINTER_STATUS_WARMING_UP) return "warming-up";
    if (status & PRINTER_STATUS_TONER_LOW) return "toner-low";
    if (status & PRINTER_STATUS_NO_TONER) return "no-toner";
    if (status & PRINTER_STATUS_PAGE_PUNT) return "page-punt";
    if (status & PRINTER_STATUS_USER_INTERVENTION) return "user-intervention";
    if (status & PRINTER_STATUS_OUT_OF_MEMORY) return "out-of-memory";
    if (status & PRINTER_STATUS_DOOR_OPEN) return "door-open";
    return "ready";
}
#else
std::string GetPrinterStatus(ipp_pstate_t state) {
    switch (state) {
        case IPP_PRINTER_IDLE: 
            return "ready";
        case IPP_PRINTER_PROCESSING: 
            return "printing";
        case IPP_PRINTER_STOPPED: 
            return "stopped";
        default: 
            return "unknown";
    }
}
#endif

PrinterInfo GetPrinterDetails(const std::string& printerName, bool isDefault = false) {
    PrinterInfo info;
    info.name = printerName;
    info.isDefault = isDefault;

    #ifdef _WIN32
    HANDLE hPrinter;
    if (OpenPrinter((LPSTR)printerName.c_str(), &hPrinter, NULL)) {
        DWORD needed;
        GetPrinter(hPrinter, 2, NULL, 0, &needed);
        
        if (needed > 0) {
            BYTE* buffer = new BYTE[needed];
            if (GetPrinter(hPrinter, 2, buffer, needed, &needed)) {
                PRINTER_INFO_2* pInfo = (PRINTER_INFO_2*)buffer;
                
                info.status = GetPrinterStatus(pInfo->Status);
                
                if (pInfo->pLocation) info.options["location"] = pInfo->pLocation;
                if (pInfo->pComment) info.options["comment"] = pInfo->pComment;
                if (pInfo->pDriverName) info.options["driver"] = pInfo->pDriverName;
                if (pInfo->pPortName) info.options["port"] = pInfo->pPortName;
            }
            delete[] buffer;
        }
        ClosePrinter(hPrinter);
    }
    #else
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(printerName.c_str(), NULL, num_dests, dests);
    
    if (dest != NULL) {
        // Get printer status and options
        http_t *http = httpConnect2(cupsServer(), ippPort(), NULL, AF_UNSPEC, HTTP_ENCRYPTION_IF_REQUESTED, 1, 30000, NULL);
        if (http != NULL) {
            ipp_t *request = ippNewRequest(IPP_OP_GET_PRINTER_ATTRIBUTES);
            
            // Construct proper printer URI
            char uri[HTTP_MAX_URI];
            httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL, "localhost", ippPort(), "/printers/%s", printerName.c_str());
            ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);
            
            // Request specific attributes
            static const char *attrs[] = {
                "printer-state",
                "printer-state-message",
                "printer-location",
                "printer-info",
                "printer-make-and-model",
                "printer-state-reasons",
                "device-uri"
            };
            ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "requested-attributes", 
                         sizeof(attrs) / sizeof(attrs[0]), NULL, attrs);
            
            ipp_t *response = cupsDoRequest(http, request, "/");
            if (response != NULL) {
                // Get printer state
                ipp_attribute_t *attr = ippFindAttribute(response, "printer-state", IPP_TAG_ENUM);
                if (attr != NULL) {
                    info.status = GetPrinterStatus((ipp_pstate_t)ippGetInteger(attr, 0));
                }
                
                // Get printer location
                attr = ippFindAttribute(response, "printer-location", IPP_TAG_TEXT);
                if (attr != NULL) {
                    info.options["location"] = ippGetString(attr, 0, NULL);
                }
                
                // Get printer info/description
                attr = ippFindAttribute(response, "printer-info", IPP_TAG_TEXT);
                if (attr != NULL) {
                    info.options["description"] = ippGetString(attr, 0, NULL);
                }
                
                // Get printer make and model
                attr = ippFindAttribute(response, "printer-make-and-model", IPP_TAG_TEXT);
                if (attr != NULL) {
                    info.options["model"] = ippGetString(attr, 0, NULL);
                }
                
                // Get device URI
                attr = ippFindAttribute(response, "device-uri", IPP_TAG_URI);
                if (attr != NULL) {
                    info.options["device-uri"] = ippGetString(attr, 0, NULL);
                }
                
                // Get state message
                attr = ippFindAttribute(response, "printer-state-message", IPP_TAG_TEXT);
                if (attr != NULL) {
                    info.options["state-message"] = ippGetString(attr, 0, NULL);
                }
                
                // Get state reasons
                attr = ippFindAttribute(response, "printer-state-reasons", IPP_TAG_KEYWORD);
                if (attr != NULL) {
                    int count = ippGetCount(attr);
                    std::string reasons;
                    for (int i = 0; i < count; i++) {
                        if (i > 0) reasons += ", ";
                        reasons += ippGetString(attr, i, NULL);
                    }
                    info.options["state-reasons"] = reasons;
                }
                
                ippDelete(response);
            }
            httpClose(http);
        }
        
        // Get additional options from destination
        for (int i = 0; i < dest->num_options; i++) {
            info.options[dest->options[i].name] = dest->options[i].value;
        }
    }
    cupsFreeDests(num_dests, dests);
    #endif

    return info;
}

class GetPrintersWorker : public Napi::AsyncWorker {
private:
    std::vector<PrinterInfo> printers;

public:
    GetPrintersWorker(Napi::Function& callback)
        : Napi::AsyncWorker(callback) {}

    void Execute() override {
        #ifdef _WIN32
        DWORD needed, returned;
        EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &needed, &returned);
        
        if (needed > 0) {
            BYTE* buffer = new BYTE[needed];
            if (EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, buffer, needed, &needed, &returned)) {
                PRINTER_INFO_2* pPrinterEnum = (PRINTER_INFO_2*)buffer;
                
                char defaultPrinter[256];
                DWORD size = sizeof(defaultPrinter);
                GetDefaultPrinter(defaultPrinter, &size);
                std::string defaultPrinterName(defaultPrinter);

                for (DWORD i = 0; i < returned; i++) {
                    std::string printerName(pPrinterEnum[i].pPrinterName);
                    printers.push_back(GetPrinterDetails(printerName, printerName == defaultPrinterName));
                }
            }
            delete[] buffer;
        }
        #else
        cups_dest_t *dests;
        int num_dests = cupsGetDests(&dests);
        
        for (int i = 0; i < num_dests; i++) {
            std::string printerName(dests[i].name);
            printers.push_back(GetPrinterDetails(printerName, dests[i].is_default));
        }
        
        cupsFreeDests(num_dests, dests);
        #endif
    }

    void OnOK() override {
        Napi::HandleScope scope(Env());
        Napi::Array printersArray = Napi::Array::New(Env(), printers.size());
        
        for (size_t i = 0; i < printers.size(); i++) {
            Napi::Object printerObj = Napi::Object::New(Env());
            printerObj.Set("name", printers[i].name);
            printerObj.Set("isDefault", printers[i].isDefault);
            printerObj.Set("status", printers[i].status);

            Napi::Object optionsObj = Napi::Object::New(Env());
            for (const auto& option : printers[i].options) {
                optionsObj.Set(option.first, option.second);
            }
            printerObj.Set("options", optionsObj);

            printersArray[i] = printerObj;
        }
        
        Callback().Call({Env().Null(), printersArray});
    }
};

class GetDefaultPrinterWorker : public Napi::AsyncWorker {
private:
    PrinterInfo printer;

public:
    GetDefaultPrinterWorker(Napi::Function& callback)
        : Napi::AsyncWorker(callback) {}

    void Execute() override {
        #ifdef _WIN32
        char defaultPrinter[256];
        DWORD size = sizeof(defaultPrinter);
        
        if (GetDefaultPrinter(defaultPrinter, &size)) {
            printer = GetPrinterDetails(defaultPrinter, true);
        } else {
            SetError("Failed to get default printer");
        }
        #else
        cups_dest_t *dests;
        int num_dests = cupsGetDests(&dests);
        const char* defaultPrinter = cupsGetDefault();
        
        if (defaultPrinter != NULL) {
            printer = GetPrinterDetails(defaultPrinter, true);
        } else {
            SetError("No default printer found");
        }
        
        cupsFreeDests(num_dests, dests);
        #endif
    }

    void OnOK() override {
        Napi::HandleScope scope(Env());
        
        Napi::Object printerObj = Napi::Object::New(Env());
        printerObj.Set("name", printer.name);
        printerObj.Set("isDefault", printer.isDefault);
        printerObj.Set("status", printer.status);

        Napi::Object optionsObj = Napi::Object::New(Env());
        for (const auto& option : printer.options) {
            optionsObj.Set(option.first, option.second);
        }
        printerObj.Set("options", optionsObj);
        
        Callback().Call({Env().Null(), printerObj});
    }
};

class PrintDirectWorker : public Napi::AsyncWorker {
private:
    std::string printerName;
    std::vector<uint8_t> data;
    std::string dataType;
    std::string result;

public:
    PrintDirectWorker(Napi::Function& callback, std::string printer, std::vector<uint8_t> printData, std::string type = "RAW")
        : Napi::AsyncWorker(callback), printerName(printer), data(printData), dataType(type) {}

    void Execute() override {
        #ifdef _WIN32
        HANDLE printerHandle;
        if (!OpenPrinter((LPSTR)printerName.c_str(), &printerHandle, NULL)) {
            SetError("Failed to open printer");
            return;
        }

        DOC_INFO_1 docInfo;
        docInfo.pDocName = (LPSTR)"Node.js Print Job";
        docInfo.pOutputFile = NULL;
        docInfo.pDatatype = (LPSTR)dataType.c_str();

        if (StartDocPrinter(printerHandle, 1, (LPBYTE)&docInfo)) {
            StartPagePrinter(printerHandle);

            DWORD bytesWritten;
            WritePrinter(printerHandle, data.data(), data.size(), &bytesWritten);
            EndPagePrinter(printerHandle);
            EndDocPrinter(printerHandle);
            result = "Print job created successfully";
        } else {
            SetError("Failed to start document printing");
        }

        ClosePrinter(printerHandle);
        #else
        int jobId = 0;
        cups_dest_t *dests;
        int num_dests = cupsGetDests(&dests);
        cups_dest_t *dest = cupsGetDest(printerName.c_str(), NULL, num_dests, dests);
        
        if (dest != NULL) {
            int num_options = 0;
            cups_option_t *options = NULL;
            
            // Convert data to char* for CUPS API
            const char* printData = reinterpret_cast<const char*>(data.data());
            size_t dataSize = data.size();
            
            jobId = cupsCreateJob(CUPS_HTTP_DEFAULT, printerName.c_str(), "Node.js Print Job", num_options, options);
            
            if (jobId > 0) {
                http_status_t httpStatus = cupsStartDocument(CUPS_HTTP_DEFAULT, printerName.c_str(), jobId, "document", dataType.c_str(), 1);
                
                if (httpStatus == HTTP_STATUS_CONTINUE) {
                    if (cupsWriteRequestData(CUPS_HTTP_DEFAULT, printData, dataSize) != HTTP_STATUS_CONTINUE) {
                        SetError("Failed to write print data");
                        cupsFinishDocument(CUPS_HTTP_DEFAULT, printerName.c_str());
                        return;
                    }
                    
                    httpStatus = static_cast<http_status_t>(cupsFinishDocument(CUPS_HTTP_DEFAULT, printerName.c_str()));
                    if (httpStatus == HTTP_STATUS_OK) {
                        result = "Print job created successfully";
                    } else {
                        SetError("Failed to finish print job");
                    }
                } else {
                    SetError("Failed to start print job");
                }
            } else {
                SetError("Failed to create print job");
            }
        } else {
            SetError("Printer not found");
        }
        
        cupsFreeDests(num_dests, dests);
        #endif
    }

    void OnOK() override {
        Napi::HandleScope scope(Env());
        Callback().Call({Env().Null(), Napi::String::New(Env(), result)});
    }
};

Napi::Value PrintDirect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string printerName;
    std::vector<uint8_t> data;
    std::string dataType = "RAW"; 

    if (info.Length() == 1 && info[0].IsObject()) {
        Napi::Object options = info[0].As<Napi::Object>();
        
        if (!options.Has("printerName") || !options.Has("data")) {
            Napi::TypeError::New(env, "Object must contain printerName and data properties")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        printerName = options.Get("printerName").As<Napi::String>().Utf8Value();
        
        if (options.Has("dataType") && options.Get("dataType").IsString()) {
            dataType = options.Get("dataType").As<Napi::String>().Utf8Value();
        }

        Napi::Value dataValue = options.Get("data");
        if (dataValue.IsString()) {
            std::string dataStr = dataValue.As<Napi::String>().Utf8Value();
            data.assign(dataStr.begin(), dataStr.end());
        } else if (dataValue.IsBuffer()) {
            Napi::Buffer<uint8_t> dataBuffer = dataValue.As<Napi::Buffer<uint8_t>>();
            data = std::vector<uint8_t>(dataBuffer.Data(), dataBuffer.Data() + dataBuffer.Length());
        } else {
            Napi::TypeError::New(env, "data must be string or buffer").ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    else if (info.Length() >= 2 && info[0].IsString() && (info[1].IsString() || info[1].IsBuffer())) {
        printerName = info[0].As<Napi::String>().Utf8Value();
        
        if (info.Length() >= 3 && info[2].IsString()) {
            dataType = info[2].As<Napi::String>().Utf8Value();
        }

        if (info[1].IsString()) {
            std::string dataStr = info[1].As<Napi::String>().Utf8Value();
            data.assign(dataStr.begin(), dataStr.end());
        } else {
            Napi::Buffer<uint8_t> dataBuffer = info[1].As<Napi::Buffer<uint8_t>>();
            data = std::vector<uint8_t>(dataBuffer.Data(), dataBuffer.Data() + dataBuffer.Length());
        }
    } else {
        Napi::TypeError::New(env, "Expected either an options object {printerName, data, [dataType]} or at least two arguments: printerName (string), data (string or buffer), [dataType (string)]")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    
    Napi::Function callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return env.Undefined();
    });
    
    PrintDirectWorker* worker = new PrintDirectWorker(callback, printerName, data, dataType);
    worker->Queue();
    
    return deferred.Promise();
}

Napi::Value GetPrinters(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    
    Napi::Function callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return env.Undefined();
    });
    
    GetPrintersWorker* worker = new GetPrintersWorker(callback);
    worker->Queue();
    
    return deferred.Promise();
}

Napi::Value GetSystemDefaultPrinter(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    
    Napi::Function callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return env.Undefined();
    });
    
    GetDefaultPrinterWorker* worker = new GetDefaultPrinterWorker(callback);
    worker->Queue();
    
    return deferred.Promise();
}

class GetStatusPrinterWorker : public Napi::AsyncWorker {
private:
    std::string printerName;
    PrinterInfo printer;

public:
    GetStatusPrinterWorker(Napi::Function& callback, std::string name)
        : Napi::AsyncWorker(callback), printerName(name) {}

    void Execute() override {
        printer = GetPrinterDetails(printerName);
        if (printer.name.empty()) {
            SetError("Printer not found");
        }
    }

    void OnOK() override {
        Napi::HandleScope scope(Env());
        
        Napi::Object printerObj = Napi::Object::New(Env());
        printerObj.Set("name", printer.name);
        printerObj.Set("isDefault", printer.isDefault);
        printerObj.Set("status", printer.status);

        Napi::Object optionsObj = Napi::Object::New(Env());
        for (const auto& option : printer.options) {
            optionsObj.Set(option.first, option.second);
        }
        printerObj.Set("options", optionsObj);
        
        Callback().Call({Env().Null(), printerObj});
    }
};

Napi::Value GetStatusPrinter(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected an object with printerName property")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object options = info[0].As<Napi::Object>();
    
    if (!options.Has("printerName") || !options.Get("printerName").IsString()) {
        Napi::TypeError::New(env, "Object must contain printerName as string")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string printerName = options.Get("printerName").As<Napi::String>().Utf8Value();
    
    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);
    
    Napi::Function callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return env.Undefined();
    });
    
    GetStatusPrinterWorker* worker = new GetStatusPrinterWorker(callback, printerName);
    worker->Queue();
    
    return deferred.Promise();
}
