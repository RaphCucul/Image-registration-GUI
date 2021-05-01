#ifndef ADMINCHECK_H
#define ADMINCHECK_H

#include <windows.h>
#include <exception>

/**
 * @file adminCheck.h
 * @brief Checks if the program was started with admin privileges or not.
 *
 * It is used to determine, if HDD usage graph should be added to the program or not.
 */


/**
 * @brief checks the token of the calling thread to see if the caller belongs to the Administrators group.
 * @return TRUE if the caller is an administrator on the local machine. Otherwise, FALSE.
 * @cite Based on code from KB #Q118626, at http://support.microsoft.com/kb/118626
 */
BOOL IsCurrentUserLocalAdministrator()
{
    BOOL   fReturn         = FALSE;
    DWORD  dwStatus        = 0;
    DWORD  dwAccessMask    = 0;
    DWORD  dwAccessDesired = 0;
    DWORD  dwACLSize       = 0;
    DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
    PACL   pACL            = NULL;
    PSID   psidAdmin       = NULL;

    HANDLE hToken              = NULL;
    HANDLE hImpersonationToken = NULL;

    PRIVILEGE_SET   ps;
    GENERIC_MAPPING GenericMapping;

    PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

    // Determine if the current thread is running as a user that is a member
    // of the local admins group.  To do this, create a security descriptor
    // that has a DACL which has an ACE that allows only local administrators
    // access.  Then, call AccessCheck with the current thread's token and
    // the security descriptor.  It will say whether the user could access an
    // object if it had that security descriptor.  Note: you do not need to
    // actually create the object.  Just checking access against the
    // security descriptor alone will be sufficient.

    const DWORD ACCESS_READ  = 1;
    const DWORD ACCESS_WRITE = 2;

    try
    {
        // AccessCheck() requires an impersonation token.  We first get a
        // primary token and then create a duplicate impersonation token.
        // The impersonation token is not actually assigned to the thread, but
        // is used in the call to AccessCheck.  Thus, this function itself never
        // impersonates, but does use the identity of the thread.  If the thread
        // was impersonating already, this function uses that impersonation
        // context.
        if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE|TOKEN_QUERY, TRUE, &hToken))
        {
            if (GetLastError() != ERROR_NO_TOKEN)
                goto Cleanup;

            if (!OpenProcessToken(GetCurrentProcess(),
                TOKEN_DUPLICATE|TOKEN_QUERY, &hToken))
                goto Cleanup;

        }

        if (!DuplicateToken (hToken, SecurityImpersonation, &hImpersonationToken))
            goto Cleanup;

        // Create the binary representation of the well-known SID that
        // represents the local administrators group.  Then create the
        // security descriptor and DACL with an ACE that allows only local
        // admins access.  After that, perform the access check.  This will
        // determine whether the current user is a local admin.
        if (!AllocateAndInitializeSid(&SystemSidAuthority, 2,
                                      SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                      0, 0, 0, 0, 0, 0, &psidAdmin))
            goto Cleanup;

        psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (psdAdmin == NULL)
            goto Cleanup;

        if (!InitializeSecurityDescriptor(psdAdmin, SECURITY_DESCRIPTOR_REVISION))
            goto Cleanup;

        // Compute size needed for the ACL.
        dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                    GetLengthSid(psidAdmin) - sizeof(DWORD);

        pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
        if (pACL == NULL)
            goto Cleanup;

        if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
            goto Cleanup;

        dwAccessMask= ACCESS_READ | ACCESS_WRITE;

        if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, psidAdmin))
            goto Cleanup;

        if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
            goto Cleanup;

        // AccessCheck validates a security descriptor somewhat; set the group
        // and owner so that enough of the security descriptor is filled out to
        // make AccessCheck happy.

        SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE);
        SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE);

        if (!IsValidSecurityDescriptor(psdAdmin))
            goto Cleanup;

        dwAccessDesired = ACCESS_READ;

        // Initialize GenericMapping structure even though you
        // do not use generic rights.
        GenericMapping.GenericRead    = ACCESS_READ;
        GenericMapping.GenericWrite   = ACCESS_WRITE;
        GenericMapping.GenericExecute = 0;
        GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;

        if (!AccessCheck(psdAdmin, hImpersonationToken, dwAccessDesired,
                         &GenericMapping, &ps, &dwStructureSize, &dwStatus,
                         &fReturn))
        {
            goto Cleanup;
        }
    }
    catch (std::exception& e) {
        goto Cleanup;
    }

    Cleanup:
        // Clean up.
        if (pACL)
            LocalFree(pACL);
        if (psdAdmin)
            LocalFree(psdAdmin);
        if (psidAdmin)
            FreeSid(psidAdmin);
        if (hImpersonationToken)
            CloseHandle (hImpersonationToken);
        if (hToken)
            CloseHandle (hToken);

    return fReturn;
}

#endif // ADMINCHECK_H
