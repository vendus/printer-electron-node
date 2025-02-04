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

struct PrinterInfo
{
    std::string name;
    bool isDefault;
    std::map<std::string, std::string> details;
    std::string status;
};

#ifdef _WIN32
std::string GetPrinterStatus(DWORD status)
{
    if (status & PRINTER_STATUS_OFFLINE)
        return "offline";
    if (status & PRINTER_STATUS_ERROR)
        return "error";
    if (status & PRINTER_STATUS_PAPER_JAM)
        return "paper-jam";
    if (status & PRINTER_STATUS_PAPER_OUT)
        return "paper-out";
    if (status & PRINTER_STATUS_MANUAL_FEED)
        return "manual-feed";
    if (status & PRINTER_STATUS_PAPER_PROBLEM)
        return "paper-problem";
    if (status & PRINTER_STATUS_BUSY)
        return "busy";
    if (status & PRINTER_STATUS_PRINTING)
        return "printing";
    if (status & PRINTER_STATUS_OUTPUT_BIN_FULL)
        return "output-bin-full";
    if (status & PRINTER_STATUS_NOT_AVAILABLE)
        return "not-available";
    if (status & PRINTER_STATUS_WAITING)
        return "waiting";
    if (status & PRINTER_STATUS_PROCESSING)
        return "processing";
    if (status & PRINTER_STATUS_INITIALIZING)
        return "initializing";
    if (status & PRINTER_STATUS_WARMING_UP)
        return "warming-up";
    if (status & PRINTER_STATUS_TONER_LOW)
        return "toner-low";
    if (status & PRINTER_STATUS_NO_TONER)
        return "no-toner";
    if (status & PRINTER_STATUS_PAGE_PUNT)
        return "page-punt";
    if (status & PRINTER_STATUS_USER_INTERVENTION)
        return "user-intervention";
    if (status & PRINTER_STATUS_OUT_OF_MEMORY)
        return "out-of-memory";
    if (status & PRINTER_STATUS_DOOR_OPEN)
        return "door-open";
    return "ready";
}
#else
std::string GetPrinterStatus(ipp_pstate_t state)
{
    switch (state)
    {
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

PrinterInfo GetPrinterDetails(const std::string &printerName, bool isDefault = false)
{
    PrinterInfo info;
    info.name = printerName;
    info.isDefault = isDefault;

#ifdef _WIN32
    HANDLE hPrinter;
    std::wstring wPrinterName(printerName.begin(), printerName.end());
    if (OpenPrinterW((LPWSTR)wPrinterName.c_str(), &hPrinter, NULL))
    {
        DWORD needed;
        GetPrinterW(hPrinter, 2, NULL, 0, &needed);
        if (needed > 0)
        {
            BYTE *buffer = new BYTE[needed];
            if (GetPrinterW(hPrinter, 2, buffer, needed, &needed))
            {
                PRINTER_INFO_2W *pInfo = (PRINTER_INFO_2W *)buffer;
                info.status = GetPrinterStatus(pInfo->Status);

                if (pInfo->pLocation)
                {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pInfo->pLocation, -1, NULL, 0, NULL, NULL);
                    if (len > 0)
                    {
                        std::vector<char> location(len);
                        WideCharToMultiByte(CP_UTF8, 0, pInfo->pLocation, -1, &location[0], len, NULL, NULL);
                        info.details["location"] = std::string(location.data());
                    }
                }

                if (pInfo->pComment)
                {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pInfo->pComment, -1, NULL, 0, NULL, NULL);
                    if (len > 0)
                    {
                        std::vector<char> comment(len);
                        WideCharToMultiByte(CP_UTF8, 0, pInfo->pComment, -1, &comment[0], len, NULL, NULL);
                        info.details["comment"] = std::string(comment.data());
                    }
                }

                if (pInfo->pDriverName)
                {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pInfo->pDriverName, -1, NULL, 0, NULL, NULL);
                    if (len > 0)
                    {
                        std::vector<char> driver(len);
                        WideCharToMultiByte(CP_UTF8, 0, pInfo->pDriverName, -1, &driver[0], len, NULL, NULL);
                        info.details["driver"] = std::string(driver.data());
                    }
                }

                if (pInfo->pPortName)
                {
                    int len = WideCharToMultiByte(CP_UTF8, 0, pInfo->pPortName, -1, NULL, 0, NULL, NULL);
                    if (len > 0)
                    {
                        std::vector<char> port(len);
                        WideCharToMultiByte(CP_UTF8, 0, pInfo->pPortName, -1, &port[0], len, NULL, NULL);
                        info.details["port"] = std::string(port.data());
                    }
                }
            }
            delete[] buffer;
        }
        ClosePrinter(hPrinter);
    }
#else
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);
    cups_dest_t *dest = cupsGetDest(printerName.c_str(), NULL, num_dests, dests);

    if (dest != NULL)
    {
        // Get options from destination
        for (int i = 0; i < dest->num_options; i++)
        {
            info.details[dest->options[i].name] = dest->options[i].value;
        }

        http_t *http = httpConnect2(cupsServer(), ippPort(), NULL, AF_UNSPEC, HTTP_ENCRYPTION_IF_REQUESTED, 1, 30000, NULL);
        if (http != NULL)
        {
            ipp_t *request = ippNewRequest(IPP_OP_GET_PRINTER_ATTRIBUTES);

            char uri[HTTP_MAX_URI];
            httpAssembleURIf(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", NULL, "localhost", ippPort(), "/printers/%s", printerName.c_str());
            ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri", NULL, uri);

            static const char *attrs[] = {
                "printer-state",
                "printer-state-message",
                "printer-location",
                "printer-info",
                "printer-make-and-model",
                "printer-state-reasons",
                "device-uri",
                "printer-uri-supported",
                "printer-type",
                "printer-name"};
            ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD, "requested-attributes",
                          sizeof(attrs) / sizeof(attrs[0]), NULL, attrs);

            ipp_t *response = cupsDoRequest(http, request, "/");
            if (response != NULL)
            {
                ipp_attribute_t *attr = ippFindAttribute(response, "printer-state", IPP_TAG_ENUM);
                if (attr != NULL)
                {
                    info.status = GetPrinterStatus((ipp_pstate_t)ippGetInteger(attr, 0));
                }

                attr = ippFindAttribute(response, "printer-location", IPP_TAG_TEXT);
                if (attr != NULL)
                {
                    info.details["location"] = ippGetString(attr, 0, NULL);
                }
                else
                {
                    info.details["location"] = "";
                }

                attr = ippFindAttribute(response, "printer-info", IPP_TAG_TEXT);
                if (attr != NULL)
                {
                    info.details["comment"] = ippGetString(attr, 0, NULL);
                }
                else
                {
                    info.details["comment"] = "";
                }

                attr = ippFindAttribute(response, "printer-make-and-model", IPP_TAG_TEXT);
                if (attr != NULL)
                {
                    info.details["driver"] = ippGetString(attr, 0, NULL);
                }
                else
                {
                    info.details["driver"] = "";
                }

                attr = ippFindAttribute(response, "device-uri", IPP_TAG_URI);
                if (attr != NULL)
                {
                    const char *uri = ippGetString(attr, 0, NULL);
                    std::string uriStr(uri);
                    size_t lastSlash = uriStr.find_last_of("/");
                    if (lastSlash != std::string::npos)
                    {
                        info.details["port"] = uriStr.substr(lastSlash + 1);
                    }
                    else
                    {
                        info.details["port"] = uri;
                    }
                }
                else
                {
                    info.details["port"] = printerName;
                }

                ippDelete(response);
            }
            httpClose(http);
        }
    }
    cupsFreeDests(num_dests, dests);
#endif

    return info;
}

class GetPrintersWorker : public Napi::AsyncWorker
{
private:
    std::vector<PrinterInfo> printers;

public:
    GetPrintersWorker(Napi::Function &callback)
        : Napi::AsyncWorker(callback) {}

    void Execute() override
    {
#ifdef _WIN32
        DWORD needed = 0;
        DWORD returned = 0;

        EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &needed, &returned);

        if (needed > 0)
        {
            BYTE *buffer = new BYTE[needed];
            if (EnumPrintersW(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 2, buffer, needed, &needed, &returned))
            {
                PRINTER_INFO_2W *pPrinterEnum = (PRINTER_INFO_2W *)buffer;
                wchar_t defaultPrinter[256];
                DWORD size = sizeof(defaultPrinter) / sizeof(defaultPrinter[0]);
                GetDefaultPrinterW(defaultPrinter, &size);
                std::wstring defaultPrinterName(defaultPrinter);
                for (DWORD i = 0; i < returned; i++)
                {
                    std::wstring printerName(pPrinterEnum[i].pPrinterName);
                    bool isDefault = (printerName == defaultPrinterName);
                    printers.push_back(GetPrinterDetails(std::string(printerName.begin(), printerName.end()), isDefault));
                }
            }
            delete[] buffer;
        }
#else
        cups_dest_t *dests;
        int num_dests = cupsGetDests(&dests);

        for (int i = 0; i < num_dests; i++)
        {
            std::string printerName(dests[i].name);
            printers.push_back(GetPrinterDetails(printerName, dests[i].is_default));
        }

        cupsFreeDests(num_dests, dests);
#endif
    }

    void OnOK() override
    {
        Napi::HandleScope scope(Env());
        Napi::Array printersArray = Napi::Array::New(Env(), printers.size());

        for (size_t i = 0; i < printers.size(); i++)
        {
            Napi::Object printerObj = Napi::Object::New(Env());
            printerObj.Set("name", printers[i].name);
            printerObj.Set("isDefault", printers[i].isDefault);
            printerObj.Set("status", printers[i].status);

            Napi::Object detailsObj = Napi::Object::New(Env());
            for (const auto &option : printers[i].details)
            {
                detailsObj.Set(option.first, option.second);
            }
            printerObj.Set("details", detailsObj);

            printersArray[i] = printerObj;
        }

        Callback().Call({Env().Null(), printersArray});
    }
};

class GetDefaultPrinterWorker : public Napi::AsyncWorker
{
private:
    PrinterInfo printer;

public:
    GetDefaultPrinterWorker(Napi::Function &callback)
        : Napi::AsyncWorker(callback) {}

    void Execute() override
    {
#ifdef _WIN32
        wchar_t defaultPrinter[256];
        DWORD size = sizeof(defaultPrinter) / sizeof(defaultPrinter[0]);
        if (GetDefaultPrinterW(defaultPrinter, &size))
        {
            printer = GetPrinterDetails(std::string(defaultPrinter, defaultPrinter + wcslen(defaultPrinter)), true);
        }
        else
        {
            SetError("Failed to get default printer");
        }
#else
        cups_dest_t *dests;
        int num_dests = cupsGetDests(&dests);
        const char *defaultPrinter = cupsGetDefault();

        if (defaultPrinter != NULL)
        {
            printer = GetPrinterDetails(defaultPrinter, true);
        }
        else
        {
            SetError("No default printer found");
        }

        cupsFreeDests(num_dests, dests);
#endif
    }

    void OnOK() override
    {
        Napi::HandleScope scope(Env());

        Napi::Object printerObj = Napi::Object::New(Env());
        printerObj.Set("name", printer.name);
        printerObj.Set("isDefault", printer.isDefault);
        printerObj.Set("status", printer.status);

        Napi::Object detailsObj = Napi::Object::New(Env());
        for (const auto &option : printer.details)
        {
            detailsObj.Set(option.first, option.second);
        }
        printerObj.Set("details", detailsObj);

        Callback().Call({Env().Null(), printerObj});
    }
};

class PrintDirectWorker : public Napi::AsyncWorker
{
private:
    std::string printerName;
    std::vector<uint8_t> data;
    std::string dataType;
    std::string result;

public:
    PrintDirectWorker(Napi::Function &callback, std::string printer, std::vector<uint8_t> printData, std::string type = "RAW")
        : Napi::AsyncWorker(callback), printerName(printer), data(printData), dataType(type) {}

    void Execute() override
    {
#ifdef _WIN32
        HANDLE printerHandle;
        // Converter string UTF-8 para wide string
        std::wstring wPrinterName;
        int len = MultiByteToWideChar(CP_UTF8, 0, printerName.c_str(), -1, NULL, 0);
        if (len > 0) {
            wPrinterName.resize(len - 1);  // -1 porque não precisamos do null terminator
            MultiByteToWideChar(CP_UTF8, 0, printerName.c_str(), -1, &wPrinterName[0], len);
        }
        else {
            SetError("Failed to convert printer name");
            return;
        }

        // Abrir a impressora usando o nome wide
        if (!OpenPrinterW((LPWSTR)wPrinterName.c_str(), &printerHandle, NULL))
        {
            SetError("Failed to open printer");
            return;
        }

        // Converter dataType para wide string
        std::wstring wDataType;
        len = MultiByteToWideChar(CP_UTF8, 0, dataType.c_str(), -1, NULL, 0);
        if (len > 0) {
            wDataType.resize(len - 1);
            MultiByteToWideChar(CP_UTF8, 0, dataType.c_str(), -1, &wDataType[0], len);
        }

        // Criar uma cópia não constante da string do nome do documento
        static wchar_t docName[] = L"Node.js Print Job";

        DOC_INFO_1W docInfo;
        docInfo.pDocName = docName;  // Usando a cópia não constante
        docInfo.pOutputFile = NULL;
        docInfo.pDatatype = const_cast<LPWSTR>(wDataType.c_str());

        if (StartDocPrinterW(printerHandle, 1, (LPBYTE)&docInfo))
        {
            StartPagePrinter(printerHandle);
            DWORD bytesWritten;
            WritePrinter(printerHandle, data.data(), static_cast<DWORD>(data.size()), &bytesWritten);
            EndPagePrinter(printerHandle);
            EndDocPrinter(printerHandle);
            result = "Print job created successfully";
        }
        else
        {
            DWORD error = GetLastError();
            char errorMsg[256];
            sprintf_s(errorMsg, "Failed to start document printing. Error code: %lu", error);
            SetError(errorMsg);
        }

        ClosePrinter(printerHandle);
#else
        int jobId = 0;
        cups_dest_t *dests;
        int num_dests = cupsGetDests(&dests);
        cups_dest_t *dest = cupsGetDest(printerName.c_str(), NULL, num_dests, dests);

        if (dest != NULL)
        {
            int num_details = 0;
            cups_option_t *details = NULL;

            const char *printData = reinterpret_cast<const char *>(data.data());
            size_t dataSize = data.size();

            jobId = cupsCreateJob(CUPS_HTTP_DEFAULT, printerName.c_str(), "Node.js Print Job", num_details, details);

            if (jobId > 0)
            {
                http_status_t httpStatus = cupsStartDocument(CUPS_HTTP_DEFAULT, printerName.c_str(), jobId, "document", dataType.c_str(), 1);

                if (httpStatus == HTTP_STATUS_CONTINUE)
                {
                    if (cupsWriteRequestData(CUPS_HTTP_DEFAULT, printData, dataSize) != HTTP_STATUS_CONTINUE)
                    {
                        SetError("Failed to write print data");
                        cupsFinishDocument(CUPS_HTTP_DEFAULT, printerName.c_str());
                        return;
                    }

                    httpStatus = static_cast<http_status_t>(cupsFinishDocument(CUPS_HTTP_DEFAULT, printerName.c_str()));
                    if (httpStatus == HTTP_STATUS_OK)
                    {
                        result = "Print job created successfully";
                    }
                    else
                    {
                        SetError("Failed to finish print job");
                    }
                }
                else
                {
                    SetError("Failed to start print job");
                }
            }
            else
            {
                SetError("Failed to create print job");
            }
        }
        else
        {
            SetError("Printer not found");
        }

        cupsFreeDests(num_dests, dests);
#endif
    }

    void OnOK() override
    {
        Napi::HandleScope scope(Env());
        Callback().Call({Env().Null(), Napi::String::New(Env(), result)});
    }
};

Napi::Value PrintDirect(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    std::string printerName;
    std::vector<uint8_t> data;
    std::string dataType = "RAW";

    if (info.Length() == 1 && info[0].IsObject())
    {
        Napi::Object options = info[0].As<Napi::Object>();

        if (!options.Has("printerName") || !options.Has("data"))
        {
            Napi::TypeError::New(env, "Object must contain printerName and data properties")
                .ThrowAsJavaScriptException();
            return env.Null();
        }

        printerName = options.Get("printerName").As<Napi::String>().Utf8Value();

        if (options.Has("dataType") && options.Get("dataType").IsString())
        {
            dataType = options.Get("dataType").As<Napi::String>().Utf8Value();
        }

        Napi::Value dataValue = options.Get("data");
        if (dataValue.IsString())
        {
            std::string dataStr = dataValue.As<Napi::String>().Utf8Value();
            data.assign(dataStr.begin(), dataStr.end());
        }
        else if (dataValue.IsBuffer())
        {
            Napi::Buffer<uint8_t> dataBuffer = dataValue.As<Napi::Buffer<uint8_t>>();
            data = std::vector<uint8_t>(dataBuffer.Data(), dataBuffer.Data() + dataBuffer.Length());
        }
        else
        {
            Napi::TypeError::New(env, "data must be string or buffer").ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    else if (info.Length() >= 2 && info[0].IsString() && (info[1].IsString() || info[1].IsBuffer()))
    {
        printerName = info[0].As<Napi::String>().Utf8Value();

        if (info.Length() >= 3 && info[2].IsString())
        {
            dataType = info[2].As<Napi::String>().Utf8Value();
        }

        if (info[1].IsString())
        {
            std::string dataStr = info[1].As<Napi::String>().Utf8Value();
            data.assign(dataStr.begin(), dataStr.end());
        }
        else
        {
            Napi::Buffer<uint8_t> dataBuffer = info[1].As<Napi::Buffer<uint8_t>>();
            data = std::vector<uint8_t>(dataBuffer.Data(), dataBuffer.Data() + dataBuffer.Length());
        }
    }
    else
    {
        Napi::TypeError::New(env, "Expected either an options object {printerName, data, [dataType]} or at least two arguments: printerName (string), data (string or buffer), [dataType (string)]")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    Napi::Function callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo &info)
                                                  {
        Napi::Env env = info.Env();
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return env.Undefined(); });

    PrintDirectWorker *worker = new PrintDirectWorker(callback, printerName, data, dataType);
    worker->Queue();

    return deferred.Promise();
}

Napi::Value GetPrinters(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    Napi::Function callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo &info)
                                                  {
        Napi::Env env = info.Env();
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return env.Undefined(); });

    GetPrintersWorker *worker = new GetPrintersWorker(callback);
    worker->Queue();

    return deferred.Promise();
}

Napi::Value GetSystemDefaultPrinter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    Napi::Function callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo &info)
                                                  {
        Napi::Env env = info.Env();
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return env.Undefined(); });

    GetDefaultPrinterWorker *worker = new GetDefaultPrinterWorker(callback);
    worker->Queue();

    return deferred.Promise();
}

class GetStatusPrinterWorker : public Napi::AsyncWorker
{
private:
    std::string printerName;
    PrinterInfo printer;

public:
    GetStatusPrinterWorker(Napi::Function &callback, std::string printer)
        : Napi::AsyncWorker(callback), printerName(printer) {}

    void Execute() override
    {
#ifdef _WIN32
        // Primeiro, vamos verificar se esta é a impressora padrão
        wchar_t defaultPrinter[256];
        DWORD size = sizeof(defaultPrinter) / sizeof(defaultPrinter[0]);
        bool isDefault = false;

        if (GetDefaultPrinterW(defaultPrinter, &size)) 
        {
            // Converter o nome da impressora atual para wide string para comparação
            std::wstring wPrinterName;
            int len = MultiByteToWideChar(CP_UTF8, 0, printerName.c_str(), -1, NULL, 0);
            if (len > 0) {
                wPrinterName.resize(len - 1);
                MultiByteToWideChar(CP_UTF8, 0, printerName.c_str(), -1, &wPrinterName[0], len);
                isDefault = (wPrinterName == std::wstring(defaultPrinter));
            }
        }

        // Agora obtemos os detalhes da impressora
        printer = GetPrinterDetails(printerName, isDefault);
#else
        cups_dest_t *dests;
        int num_dests = cupsGetDests(&dests);
        cups_dest_t *dest = cupsGetDest(printerName.c_str(), NULL, num_dests, dests);

        if (dest != NULL)
        {
            printer = GetPrinterDetails(printerName, dest->is_default);
        }
        else
        {
            SetError("Printer not found");
        }

        cupsFreeDests(num_dests, dests);
#endif
    }

    void OnOK() override
    {
        Napi::HandleScope scope(Env());

        Napi::Object printerObj = Napi::Object::New(Env());
        printerObj.Set("name", printer.name);
        printerObj.Set("isDefault", printer.isDefault);
        printerObj.Set("status", printer.status);

        Napi::Object detailsObj = Napi::Object::New(Env());
        for (const auto &option : printer.details)
        {
            detailsObj.Set(option.first, option.second);
        }
        printerObj.Set("details", detailsObj);

        Callback().Call({Env().Null(), printerObj});
    }
};

Napi::Value GetStatusPrinter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsObject())
    {
        Napi::TypeError::New(env, "Expected an object with printerName property")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Object options = info[0].As<Napi::Object>();

    if (!options.Has("printerName") || !options.Get("printerName").IsString())
    {
        Napi::TypeError::New(env, "Object must contain printerName as string")
            .ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string printerName = options.Get("printerName").As<Napi::String>().Utf8Value();

    Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

    Napi::Function callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo &info)
                                                  {
        Napi::Env env = info.Env();
        if (info[0].IsNull()) {
            deferred.Resolve(info[1]);
        } else {
            deferred.Reject(info[0].As<Napi::Error>().Value());
        }
        return env.Undefined(); });

    GetStatusPrinterWorker *worker = new GetStatusPrinterWorker(callback, printerName);
    worker->Queue();

    return deferred.Promise();
}
