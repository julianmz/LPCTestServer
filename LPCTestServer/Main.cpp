#include <iostream>
#include <stdexcept>

#include "InterfaceConsts.hpp"
#include "LPCTestServer_h.h"
#include "Utils.hpp"

namespace {

    BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType) {

        BOOL handled = TRUE;

        switch (ctrlType) {

        case CTRL_C_EVENT:

            RpcMgmtStopServerListening(NULL);
            break;

        default:

            handled = FALSE;
        }

        return handled;
    }
}

int wmain(int argc, wchar_t* argv[]) {

    int exitCode = 1;

    try {

        if (!SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE))
            throw std::runtime_error(lpc_test::GetWin32FailMsg("SetConsoleCtrlHandler", GetLastError()));
        
        // TODO: Check for errors during console mode adjustment
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD  consoleMode   = 0;
        GetConsoleMode(consoleHandle, &consoleMode);
        SetConsoleMode(consoleHandle, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

        std::cout << "Running with admin privileges: "
                  << (lpc_test::ThreadHasAdminPrivileges() ? "TRUE" : "FALSE")
                  << std::endl;

        wchar_t protoSequence[] = LPC_PROTOCOL_SEQUENCE;
        wchar_t endpoint[]      = LPC_ENDPOINT;

        RPC_STATUS rpcStatus = RpcServerUseProtseqEp(protoSequence, RPC_C_LISTEN_MAX_CALLS_DEFAULT, endpoint, NULL);
        if (rpcStatus != RPC_S_OK)
            throw std::runtime_error(lpc_test::GetRPCFailMsg("RpcServerUseProtseqEp", rpcStatus));

        rpcStatus = RpcServerRegisterIf(LPCTestServer_v1_0_s_ifspec, NULL, NULL);
        if (rpcStatus != RPC_S_OK)
            throw std::runtime_error(lpc_test::GetRPCFailMsg("RpcServerRegisterIf", rpcStatus));

        rpcStatus = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, 0);
        if (rpcStatus != RPC_S_OK)
            throw std::runtime_error(lpc_test::GetRPCFailMsg("RpcServerListen", rpcStatus));

        exitCode = 0;
    }
    catch (const std::exception& exception) {
        std::cout << "Exception caught in main thread: \"" << exception.what() << "\"" << std::endl;
    }

    return exitCode;
}