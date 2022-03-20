#include <iostream>
#include <vector>

#include "../LPCTestServer/InterfaceConsts.hpp"
#include "../LPCTestServer/LPCTestServer_h.h"
#include "../LPCTestServer/Utils.hpp"

namespace {

    void EstablishRPCConnection(RPC_WSTR* rpcStrBinding, RPC_BINDING_HANDLE* rpcBindingHandle) {

        wchar_t interfaceUUID[] = LPC_INTERFACE_UUID;
        wchar_t protoSequence[] = LPC_PROTOCOL_SEQUENCE;
        wchar_t endpoint[]      = LPC_ENDPOINT;

        RPC_STATUS rpcStatus = RpcStringBindingCompose(
            interfaceUUID,
            protoSequence,
            NULL,
            endpoint,
            NULL,
            rpcStrBinding
        );

        if (rpcStatus != RPC_S_OK)
            throw std::runtime_error(lpc_test::GetRPCFailMsg("RpcStringBindingCompose", rpcStatus));

        rpcStatus = RpcBindingFromStringBinding(*rpcStrBinding, rpcBindingHandle);
        if (rpcStatus != RPC_S_OK)
            throw std::runtime_error(lpc_test::GetRPCFailMsg("RpcBindingFromStringBinding", rpcStatus));
    }
}

int wmain(int argc, wchar_t* argv[]) {

    int exitCode = 1;

    // TODO: Use RAII for cleanup
    RPC_WSTR           rpcStrBinding    = NULL;
    RPC_BINDING_HANDLE rpcBindingHandle = NULL;

    try {

        EstablishRPCConnection(&rpcStrBinding, &rpcBindingHandle);

        const std::vector<LPC_COLOR_E> color = { LPC_COLOR_RED, LPC_COLOR_GREEN, LPC_COLOR_BLUE };
        for (auto colorIt = color.cbegin(); colorIt != color.cend(); ++colorIt) {
            
            RPC_STATUS rpcStatus = lpc_set_color(rpcBindingHandle, *colorIt);
            if (rpcStatus != RPC_S_OK)
                throw std::runtime_error(lpc_test::GetRPCFailMsg("lpc_set_color", rpcStatus));

            LPC_COLOR_E setColor = LPC_COLOR_RED;
            rpcStatus            = lpc_get_color(rpcBindingHandle, &setColor);
            if (rpcStatus != RPC_S_OK)
                throw std::runtime_error(lpc_test::GetRPCFailMsg("lpc_get_color", rpcStatus));

            if (setColor != *colorIt)
                throw std::runtime_error("Unexpected set color");

            rpcStatus = lpc_print_msg(rpcBindingHandle, "This is a colored message (hopefully)");
            if (rpcStatus != RPC_S_OK)
                throw std::runtime_error(lpc_test::GetRPCFailMsg("lpc_print_msg", rpcStatus));
        }

        exitCode = 0;
    }
    catch (const std::exception& exception) {
        std::cout << "Exception caught in main thread: \"" << exception.what() << "\"" << std::endl;
    }

    if (rpcStrBinding)
        RpcStringFree(&rpcStrBinding);

    if (rpcBindingHandle)
        RpcBindingFree(&rpcBindingHandle);

    return exitCode;
}

extern "C" {

    void __RPC_FAR* __RPC_USER midl_user_allocate(size_t cBytes)
    {
        return((void __RPC_FAR*) malloc(cBytes));
    }

    void __RPC_USER midl_user_free(void __RPC_FAR* p)
    {
        free(p);
    }
}