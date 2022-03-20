#include <exception>
#include <functional>
#include <iostream>
#include <mutex>

#include <Windows.h>
#include <rpc.h>

#include "LPCTestServer_h.h"
#include "Utils.hpp"

namespace {

    std::mutex  mtx;
    LPC_COLOR_E activeColor = LPC_COLOR_RED;

    const char* GetActiveColorCodeStr() {

        const char* colorCodeStr;

        switch (activeColor) {

        case LPC_COLOR_RED:

            colorCodeStr = "31";
            break;

        case LPC_COLOR_GREEN:

            colorCodeStr = "32";
            break;

        case LPC_COLOR_BLUE:

            colorCodeStr = "34";
            break;

        default:

            colorCodeStr = "37"; // White
        }

        return colorCodeStr;
    }

    error_status_t CallGuarded(handle_t binding, const std::function<error_status_t()>& func) noexcept {

        error_status_t status              = RpcImpersonateClient(binding);
        const bool     impersonationActive = status == RPC_S_OK;

        if      (!impersonationActive)
            std::cout << "Impersonation has failed" << std::endl;
        else if (!lpc_test::ThreadHasAdminPrivileges()) {
            
            // NOTE: Checking for admin privileges is purely optional, remove this check whenever you want
            std::cout << "RPC client does not have admin privileges"
                      << std::endl;
            
            status = RPC_S_ACCESS_DENIED;
        }
        else {

            try {
                status = func();
            }
            catch (const std::exception& exception) {
                std::cout << "Exception caught by guard: \"" << exception.what() << "\"" << std::endl;
            }

            status = RpcRevertToSelf();
            if (status != RPC_S_OK)
                std::cout << "Impersonation could not be reverted" << std::endl;
        }

        return status;
    }

    error_status_t DoSetColor(handle_t binding, LPC_COLOR_E color) {

        const std::lock_guard<std::mutex> lockMtx(mtx);
        
        activeColor = color;
        
        std::cout << "(color set)" << std::endl;
        return RPC_S_OK;
    }

    error_status_t DoGetColor(handle_t binding, LPC_COLOR_E* color) {

        const std::lock_guard<std::mutex> lockMtx(mtx);
        
        *color = activeColor;
        
        std::cout << "(color get)" << std::endl;
        return RPC_S_OK;
    }

    error_status_t DoPrintMSG(handle_t binding, const char* msg) {

        const std::lock_guard<std::mutex> lockMtx(mtx);
        
        std::cout << "\033[1;" << GetActiveColorCodeStr() << "m"
                  << msg
                  << "\033[0m"
                  << std::endl;

        return RPC_S_OK;
    }
}

extern "C" {

    error_status_t lpc_set_color(handle_t binding, LPC_COLOR_E color) {

        return CallGuarded(binding, [=]() { return DoSetColor(binding, color); });
    }

    error_status_t lpc_get_color(handle_t binding, LPC_COLOR_E* color) {

        return CallGuarded(binding, [=]() { return DoGetColor(binding, color); });
    }

    error_status_t lpc_print_msg(handle_t binding, const char* msg) {

        return CallGuarded(binding, [=]() { return DoPrintMSG(binding, msg); });
    }

    void __RPC_FAR* __RPC_USER midl_user_allocate(size_t cBytes)
    {
        return((void __RPC_FAR*) malloc(cBytes));
    }

    void __RPC_USER midl_user_free(void __RPC_FAR* p)
    {
        free(p);
    }
}