/*
 *  OpenVPN-GUI -- A Windows GUI for OpenVPN.
 *
 *  Copyright (C) 2009 Heiko Hund <heikoh@users.sf.net>
 *                2011 Michael Berger <michael.berger@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program (see the file COPYING included with this
 *  distribution); if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define _WIN32_IE 0x0500

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <prsht.h>
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>

#include "config.h"
#include "main.h"
#include "localization.h"
#include "openvpn-gui-res.h"
#include "options.h"
#include "registry.h"
#include "gui.h"

extern options_t o;

static const LANGID fallbackLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL);
static LANGID gui_language;

static HRSRC
FindResourceLang(PTSTR resType, PTSTR resId, LANGID langId)
{
    HRSRC res;

    /* try to find the resource in requested language */
    res = FindResourceEx(o.hInstance, resType, resId, langId);
    if (res)
        return res;

    /* try to find the resource in the default language */
    res = FindResourceEx(o.hInstance, resType, resId, fallbackLangId);
    if (res)
        return res;

    /* try to find the resource in any language */
    return FindResource(o.hInstance, resId, resType);
}


static LANGID
GetGUILanguage(void)
{
    if (gui_language != 0)
        return gui_language;

    HKEY regkey;
    DWORD value = 0;

    LONG status = RegOpenKeyEx(HKEY_CURRENT_USER, GUI_REGKEY_HKCU, 0, KEY_READ, &regkey);
    if (status == ERROR_SUCCESS)
        GetRegistryValueNumeric(regkey, _T("ui_language"), &value);

    gui_language = (LANGID) ( value != 0 ? value : LANGIDFROMLCID(GetSystemDefaultLCID()) );
    InitMUILanguage(gui_language);
    return gui_language;
}


static void
SetGUILanguage(LANGID langId)
{
    HKEY regkey;
    if (RegCreateKeyEx(HKEY_CURRENT_USER, GUI_REGKEY_HKCU, 0, NULL, 0,
        KEY_WRITE, NULL, &regkey, NULL) != ERROR_SUCCESS )
        ShowLocalizedMsg(IDS_ERR_CREATE_REG_HKCU_KEY, GUI_REGKEY_HKCU);

    SetRegistryValueNumeric(regkey, _T("ui_language"), langId);
    InitMUILanguage(langId);
    gui_language = langId;
}


static int
LoadStringLang(UINT stringId, LANGID langId, PTSTR buffer, int bufferSize, va_list args)
{
    PWCH entry;
    PTSTR resBlockId = MAKEINTRESOURCE(stringId / 16 + 1);
    int resIndex = stringId & 15;

    /* find resource block for string */
    HRSRC res = FindResourceLang(RT_STRING, resBlockId, langId);
    if (res == NULL)
        return 0;

    /* get pointer to first entry in resource block */
    entry = (PWCH) LoadResource(o.hInstance, res);
    if (entry == NULL)
        return 0;

    /* search for string in block */
    for (int i = 0; i < 16; i++)
    {
        /* skip over this entry */
        if (i != resIndex)
        {
            entry += ((*entry) + 1);
            continue;
        }

        /* string does not exist */
        if (i == resIndex && *entry == 0)
            break;

        /* string found, copy it */
        PTSTR formatStr = (PTSTR) malloc((*entry + 1) * sizeof(TCHAR));
        if (formatStr == NULL)
            break;
        formatStr[*entry] = 0;

        wcsncpy(formatStr, entry + 1, *entry);
        _vsntprintf(buffer, bufferSize, formatStr, args);
        buffer[bufferSize - 1] = 0;
        free(formatStr);
        return _tcslen(buffer);
    }

    return 0;
}


static PTSTR
__LoadLocalizedString(const UINT stringId, va_list args)
{
    static TCHAR msg[512];
    msg[0] = 0;
    LoadStringLang(stringId, GetGUILanguage(), msg, _tsizeof(msg), args);
    return msg;
}


PTSTR
LoadLocalizedString(const UINT stringId, ...)
{
    va_list args;
    va_start(args, stringId);
    PTSTR str = __LoadLocalizedString(stringId, args);
    va_end(args);
    return str;
}


int
LoadLocalizedStringBuf(PTSTR buffer, int bufferSize, const UINT stringId, ...)
{
    va_list args;
    va_start(args, stringId);
    int len = LoadStringLang(stringId, GetGUILanguage(), buffer, bufferSize, args);
    va_end(args);
    return len;
}


void
ShowLocalizedMsg(const UINT stringId, ...)
{
    va_list args;
    va_start(args, stringId);
    MessageBox(NULL, __LoadLocalizedString(stringId, args), _T(PACKAGE_NAME), MB_OK | MB_SETFOREGROUND);
    va_end(args);
}


HICON
LoadLocalizedIcon(const UINT iconId, const int cxDesired, const int cyDesired)
{
    LANGID langId = GetGUILanguage();

    /* find group icon resource */
    HRSRC res = FindResourceLang(RT_GROUP_ICON, MAKEINTRESOURCE(iconId), langId);
    if (res == NULL)
        return NULL;

    HGLOBAL resInfo = LoadResource(o.hInstance, res);
    if (resInfo == NULL)
        return NULL;

    int id = LookupIconIdFromDirectoryEx((PBYTE) resInfo, TRUE, cxDesired, cyDesired, LR_DEFAULTCOLOR);
    if (id == 0)
        return NULL;

    /* find the actual icon */
    res = FindResourceLang(RT_ICON, MAKEINTRESOURCE(id), langId);
    if (res == NULL)
        return NULL;

    resInfo = LoadResource(o.hInstance, res);
    if (resInfo == NULL)
        return NULL;

    DWORD resSize = SizeofResource(o.hInstance, res);
    if (resSize == 0)
        return NULL;

    PBYTE resResource = (PBYTE) LockResource(resInfo);
    if (resResource == NULL)
        return NULL;

    return CreateIconFromResourceEx(resResource, resSize, TRUE, 0x30000, cxDesired, cyDesired, LR_DEFAULTCOLOR);
}


LPCDLGTEMPLATE
LocalizedDialogResource(const UINT dialogId)
{
    /* find dialog resource */
    HRSRC res = FindResourceLang(RT_DIALOG, MAKEINTRESOURCE(dialogId), GetGUILanguage());
    if (res == NULL)
        return NULL;

    return (LPCDLGTEMPLATE) LoadResource(o.hInstance, res);
}


INT_PTR
LocalizedDialogBoxParam(const UINT dialogId, DLGPROC dialogFunc, const LPARAM param)
{
    LPCDLGTEMPLATE resInfo = LocalizedDialogResource(dialogId);
    if (resInfo == NULL)
        return -1;

    return ExtendedDialogBoxIndirectParam(o.hInstance, resInfo, NULL, dialogFunc, param);
}


INT_PTR
LocalizedDialogBox(const UINT dialogId, DLGPROC dialogFunc)
{
    return LocalizedDialogBoxParam(dialogId, dialogFunc, 0);
}


HWND
CreateLocalizedDialogParam(const UINT dialogId, DLGPROC dialogFunc, const LPARAM param)
{
    /* find dialog resource */
    HRSRC res = FindResourceLang(RT_DIALOG, MAKEINTRESOURCE(dialogId), GetGUILanguage());
    if (res == NULL)
        return NULL;

    HGLOBAL resInfo = LoadResource(o.hInstance, res);
    if (resInfo == NULL)
        return NULL;

    return ExtendedCreateDialogIndirectParam(o.hInstance, (CONST DLGTEMPLATE *) resInfo, NULL, dialogFunc, param);
}


HWND
CreateLocalizedDialog(const UINT dialogId, DLGPROC dialogFunc)
{
    return CreateLocalizedDialogParam(dialogId, dialogFunc, 0);
}


static PTSTR
LangListEntry(const UINT stringId, const LANGID langId, ...)
{
    static TCHAR str[128];
    va_list args;

    va_start(args, langId);
    LoadStringLang(stringId, langId, str, _tsizeof(str), args);
    va_end(args);
    return str;
}


typedef struct {
    HWND languages;
    LANGID language;
} langProcData;


static BOOL
FillLangListProc(HANDLE module, PTSTR type, PTSTR stringId, WORD langId, LONG_PTR lParam)
{
    langProcData *data = (langProcData*) lParam;

    int index = ComboBox_AddString(data->languages, LangListEntry(IDS_LANGUAGE_NAME, langId));
    (void) ComboBox_SetItemData(data->languages, index, langId);

    /* Select this item if it is the currently displayed language */
    if (langId == data->language
    ||  (PRIMARYLANGID(langId) == PRIMARYLANGID(data->language)
     && ComboBox_GetCurSel(data->languages) == CB_ERR) )
        (void) ComboBox_SetCurSel(data->languages, index);

    return TRUE;
}


INT_PTR CALLBACK
LanguageSettingsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HICON hIcon;
    LPPSHNOTIFY psn;
    langProcData langData;

    langData.languages = GetDlgItem(hwndDlg, ID_CMB_LANGUAGE);
    langData.language = GetGUILanguage();

    switch(msg) {

    case WM_INITDIALOG:
        hIcon = LoadLocalizedIcon(ID_ICO_APP, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
        if (hIcon)
            SendMessage(GetParent(hwndDlg), WM_SETICON, (WPARAM) (ICON_BIG), (LPARAM) (hIcon));

        hIcon = LoadLocalizedIcon(ID_ICO_APP, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
        if (hIcon)
            SendMessage(GetParent(hwndDlg), WM_SETICON, (WPARAM) (ICON_SMALL), (LPARAM) (hIcon));

        /* Populate UI language selection combo box */
        EnumResourceLanguages( NULL, RT_STRING, MAKEINTRESOURCE(IDS_LANGUAGE_NAME / 16 + 1),
            (ENUMRESLANGPROC) FillLangListProc, (LONG_PTR) &langData );

        /* If none of the available languages matched, select the fallback */
        if (ComboBox_GetCurSel(langData.languages) == CB_ERR)
            (void) ComboBox_SelectString(langData.languages, -1,
                LangListEntry(IDS_LANGUAGE_NAME, fallbackLangId));

        /* Clear language id data for the selected item */
        (void) ComboBox_SetItemData(langData.languages, ComboBox_GetCurSel(langData.languages), 0);

        /* Center the propery sheet */
        ClipOrCenterWindowToMonitor(GetParent(hwndDlg), MONITOR_CENTER | MONITOR_WORKAREA);

        /* Set the global window handle for the settings dialog */
        o.hwndSettings = GetParent(hwndDlg);

        break;

    case WM_NOTIFY:
        psn = (LPPSHNOTIFY) lParam;
        if (psn->hdr.code == (UINT) PSN_APPLY)
        {
            LANGID langId = (LANGID) ComboBox_GetItemData(langData.languages,
                ComboBox_GetCurSel(langData.languages));

            if (langId != 0)
                SetGUILanguage(langId);

            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
        break;
    }

    return FALSE;
}
