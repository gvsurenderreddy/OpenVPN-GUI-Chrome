/*
 *  OpenVPN-GUI -- A Windows GUI for OpenVPN.
 *
 *  Copyright (C) 2011 Michael Berger <michael.berger@gmx.de>
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

#ifndef GUI_H
#define GUI_H

#define TWIPS_PER_POINT 20

#define MONITOR_CENTER   0x0001        // Center rect to monitor
#define MONITOR_CLIP     0x0000        // Clip rect to monitor
#define MONITOR_WORKAREA 0x0002        // Use monitor work area
#define MONITOR_AREA     0x0000        // Use monitor entire area

BOOL IsWindowsVistaOrLater(void);
BOOL IsDialogEx(LPCDLGTEMPLATE);
INT_PTR ExtendedDialogBoxIndirectParam(const HINSTANCE, LPCDLGTEMPLATE, const HWND, const DLGPROC, const LPARAM);
HWND ExtendedCreateDialogIndirectParam(const HINSTANCE, LPCDLGTEMPLATE, const HWND, const DLGPROC, const LPARAM);
void DeleteModifiedDialogTemplate(LPCDLGTEMPLATE);
LPCDLGTEMPLATE CreateModifiedDialogTemplate(LPCDLGTEMPLATE);
DWORD GetSizeOfDialogTemplate(LPCDLGTEMPLATE);
void SkipElement(LPBYTE *);
void CopyElement(LPBYTE *, LPBYTE *);
void ReplaceFontPointSize(LPBYTE *, LPBYTE *);
void ReplaceFontTypeface(LPBYTE *, LPBYTE *);
void RemoveSysMenu(HWND);
BOOL GetControlRect(const HWND, LPRECT);
void ClipOrCenterRectToMonitor(const LPRECT, const UINT);
void ClipOrCenterWindowToMonitor(const HWND, const UINT);

#endif
