/*
 * $Id: mail.h,v 1.4 1995/07/02 01:22:47 robertj Exp $
 *
 * Portable Windows Library
 *
 * Mail Interface
 *
 * Copyright 1993 Equivalence
 *
 * $Log: mail.h,v $
 * Revision 1.4  1995/07/02 01:22:47  robertj
 * Changed mail to use CMC then MAPI if available.
 *
 * Revision 1.3  1995/06/17 00:49:16  robertj
 * Changed name to simply PMail.
 * Fixed types of MAPI calls.
 *
 * Revision 1.2  1995/04/01 08:04:40  robertj
 * Added GUI support.
 *
 * Revision 1.1  1995/03/14 12:44:51  robertj
 * Initial revision
 *
 */

#ifndef _PMAIL

#ifndef P_HAS_MAPI
#define P_HAS_MAPI 1
#endif

#ifndef P_HAS_CMC
#define P_HAS_CMC 1
#endif

#if P_HAS_MAPI
#include <mapi.h>
#endif

#if P_HAS_CMC
#include <xcmc.h>
#endif


#include "../../common/mail.h"

  protected:
    DWORD  sessionId;
    DWORD  lastError;
    UINT   hUserInterface;

#if P_HAS_CMC
    PDECLARE_CLASS(CMCDLL, PDynaLink)
      public:
        CMCDLL();

        CMC_return_code (FAR PASCAL *logon)(
            CMC_string              service,
            CMC_string              user,
            CMC_string              password,
            CMC_enum                character_set,
            CMC_ui_id               ui_id,
            CMC_uint16              caller_cmc_version,
            CMC_flags               logon_flags,
            CMC_session_id FAR      *session,
            CMC_extension FAR       *logon_extensions
        );
        CMC_return_code (FAR PASCAL *logoff)(
            CMC_session_id          session,
            CMC_ui_id               ui_id,
            CMC_flags               logoff_flags,
            CMC_extension FAR       *logoff_extensions
        );
        CMC_return_code (FAR PASCAL *free)(
            CMC_buffer              memory
        );
        CMC_return_code (FAR PASCAL *query_configuration)(
            CMC_session_id session,
            CMC_enum item,
            CMC_buffer                    reference,
            CMC_extension FAR *config_extensions
        );
        CMC_return_code (FAR PASCAL *look_up)(
            CMC_session_id          session,
            CMC_recipient FAR       *recipient_in,
            CMC_flags               look_up_flags,
            CMC_ui_id               ui_id,
            CMC_uint32 FAR          *count,
            CMC_recipient FAR * FAR *recipient_out,
            CMC_extension FAR       *look_up_extensions
        );
        CMC_return_code (FAR PASCAL *list)(
            CMC_session_id          session,
            CMC_string              message_type,
            CMC_flags               list_flags,
            CMC_message_reference   *seed,
            CMC_uint32 FAR          *count,
            CMC_ui_id               ui_id,
            CMC_message_summary FAR * FAR *result,
            CMC_extension FAR       *list_extensions
        );
        CMC_return_code (FAR PASCAL *send)(
            CMC_session_id          session,
            CMC_message FAR         *message,
            CMC_flags               send_flags,
            CMC_ui_id               ui_id,
            CMC_extension FAR       *send_extensions
        );
        CMC_return_code (FAR PASCAL *read)(
            CMC_session_id          session,
            CMC_message_reference   *message_reference,
            CMC_flags               read_flags,
            CMC_message FAR * FAR   *message,
            CMC_ui_id               ui_id,
            CMC_extension FAR       *read_extensions
        );
        CMC_return_code (FAR PASCAL *act_on)(
            CMC_session_id          session,
            CMC_message_reference   *message_reference,
            CMC_enum                operation,
            CMC_flags               act_on_flags,
            CMC_ui_id               ui_id,
            CMC_extension FAR       *act_on_extensions
        );
    };
    CMCDLL cmc;
#endif
#if P_HAS_MAPI
    PDECLARE_CLASS(MAPIDLL, PDynaLink)
      public:
        MAPIDLL();

        ULONG (FAR PASCAL *Logon)(HWND, LPCSTR, LPCSTR, FLAGS, ULONG, LPLHANDLE);
        ULONG (FAR PASCAL *Logoff)(LHANDLE, HWND, FLAGS, ULONG);
        ULONG (FAR PASCAL *SendMail)(LHANDLE, HWND, lpMapiMessage, FLAGS, ULONG);
        ULONG (FAR PASCAL *SendDocuments)(HWND, LPSTR, LPSTR, LPSTR, ULONG);
        ULONG (FAR PASCAL *FindNext)(LHANDLE, HWND, LPCSTR, LPCSTR, FLAGS, ULONG, LPSTR);
        ULONG (FAR PASCAL *ReadMail)(LHANDLE, HWND, LPCSTR, FLAGS, ULONG, lpMapiMessage FAR *);
        ULONG (FAR PASCAL *SaveMail)(LHANDLE, HWND, lpMapiMessage, FLAGS, ULONG, LPSTR);
        ULONG (FAR PASCAL *DeleteMail)(LHANDLE, HWND, LPCSTR, FLAGS, ULONG);
        ULONG (FAR PASCAL *FreeBuffer)(LPVOID);
        ULONG (FAR PASCAL *Address)(LHANDLE, HWND, LPSTR, ULONG, LPSTR, ULONG, lpMapiRecipDesc, FLAGS, ULONG, LPULONG, lpMapiRecipDesc FAR *);
        ULONG (FAR PASCAL *Details)(LHANDLE, HWND,lpMapiRecipDesc, FLAGS, ULONG);
        ULONG (FAR PASCAL *ResolveName)(LHANDLE, HWND, LPCSTR, FLAGS, ULONG, lpMapiRecipDesc FAR *);
    };
    MAPIDLL mapi;
#endif
};


#endif

// End Of File ///////////////////////////////////////////////////////////////
