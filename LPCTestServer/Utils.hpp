#pragma once

#include <iomanip>
#include <sstream>
#include <string>

#include <Windows.h>
#include <rpc.h>

namespace lpc_test {

    inline std::string GetWin32FailMsg(const char* funcName, DWORD lastErr) {

        std::stringstream sstream;
        sstream << funcName << "() has failed (" << lastErr << ")";

        return sstream.str();
    }

    inline std::string GetRPCFailMsg(const char* funcName, RPC_STATUS rpcStatus) {

        std::stringstream sstream;
        sstream << funcName
                << "() has failed (0x"
                << std::setw(8)
                << std::setfill('0')
                << std::hex
                << rpcStatus
                << ")";

        return sstream.str();
    }

    inline bool ThreadHasAdminPrivileges() noexcept {

        bool hasAdminPrivileges = false;

        BYTE  localAdminsGroupSid[SECURITY_MAX_SID_SIZE] = { 0 };
        DWORD sidLen                                     = SECURITY_MAX_SID_SIZE;

        if (CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, localAdminsGroupSid, &sidLen)) {

            BOOL isMember = FALSE;
            if (CheckTokenMembership(NULL, &localAdminsGroupSid, &isMember))
                hasAdminPrivileges = isMember == TRUE;
        }

        return hasAdminPrivileges;
    }
}
