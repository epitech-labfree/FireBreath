/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Nov 24, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#ifndef H_PLUGINWINDOWWIN
#define H_PLUGINWINDOWWIN

#include "Win/win_common.h"
#include "PluginWindow.h"

#include <map>

#define WM_ASYNCTHREADINVOKE    WM_USER + 1

namespace FB {

    class PluginWindowWin : public PluginWindow
    {
    public:
        PluginWindowWin(HWND hWnd);
        virtual ~PluginWindowWin();
        static LRESULT CALLBACK _WinProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                                          LPARAM lParam);
        HWND getHWND() { return m_hWnd; }

        void setBrowserHWND(HWND hWnd) { m_browserhWnd = hWnd; }
        HWND getBrowserHWND() { return m_browserhWnd; }
        void setCallOldWinProc(bool callOld) { m_callOldWinProc = callOld; }

        typedef std::map<void*,PluginWindowWin*> PluginWindowMap;

        virtual void InvalidateWindow();

        static HWND createMessageWindow();

    protected:
        static PluginWindowMap m_windowMap;

        bool WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParamm, LRESULT & lRes);
        WNDPROC lpOldWinProc;
        bool m_callOldWinProc;
        HWND m_hWnd;
        HWND m_browserhWnd;

        virtual bool CustomWinProc(HWND hWnd,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParamm,
                                   LRESULT & lRes) { return false; }
    };

};

#endif // H_PLUGINWINDOWWIN
