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

#ifndef H_PLUGINWINDOWX11
#define H_PLUGINWINDOWX11

#include "config.h"

#if FB_GUI_DISABLED != 1

#include <X11/Xlib.h>
#include <gtk/gtk.h>

#endif

#include "PluginWindow.h"
#include "WindowContextX11.h"

#include <map>

namespace FB {

    class PluginWindowX11 : public PluginWindow
    {
    public:
        PluginWindowX11(const WindowContextX11&);
        virtual ~PluginWindowX11();

//         int16_t HandleEvent(EventRecord* evt);
        void setWindowPosition(int x, int y, int w, int h);
        void getWindowPosition(int &x, int &y, int &w, int &h);
        void setWindowClipping(int t, int l, int b, int r);
        void getWindowClipping(int &t, int &l, int &b, int &r);
        virtual void InvalidateWindow();

#if FB_GUI_DISABLED != 1
    public:
        GdkNativeWindow getWindow();
        void setBrowserWindow(GdkNativeWindow win) {  m_browserWindow = win; }
        GdkNativeWindow getBrowserWindow() { return m_browserWindow; }
        static gboolean _EventCallback(GtkWidget *widget, GdkEvent *event, gpointer user_data);
    protected:
        gboolean EventCallback(GtkWidget *widget, GdkEvent *event);

        GdkNativeWindow m_window;
        GdkNativeWindow m_browserWindow;
        GtkWidget *m_container;
        GtkWidget *m_canvas;

#endif

    protected:
        int m_x;
        int m_y;
        int m_width;
        int m_height;

        int m_clipLeft;
        int m_clipRight;
        int m_clipTop;
        int m_clipBottom;
    };

};

#endif // H_PLUGINWINDOWX11
