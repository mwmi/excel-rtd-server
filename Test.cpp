#include "RtdServer.h"
#include <iostream>

int main(void) {
    CoInitialize(NULL); // 初始化COM库
    std::cout << "COM library initialized." << std::endl;
    RtdServer *pIcom = NULL;
    HRESULT hResult;
    // 创建COM组件，返回默认接口
    hResult = CoCreateInstance(CLSID_RtdServer, NULL, CLSCTX_INPROC_SERVER, IID_IRtdServer, (void **)&pIcom);
    if (SUCCEEDED(hResult)) {
        std::cout << "COM instance created successfully." << std::endl;

        // 调用HelloWorld方法
        int result = pIcom->test();
        std::cout << "returned: " << result << std::endl;

        // 释放接口
        pIcom->Release();
        std::cout << "COM instance released." << std::endl;
    } else {
        std::cerr << "Failed to create COM instance. Error: " << std::hex << hResult << std::endl;
    }
    CoUninitialize(); // 释放COM库
    return 0;
}