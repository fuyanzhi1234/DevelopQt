// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "cefsimple/simple_app.h"

#include <string>

#include "cefsimple/simple_handler.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

SimpleApp::SimpleApp(HWND hParent) {
	m_hParent = hParent;
}

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  // Information used when creating the native window.
  CefWindowInfo window_info;

#if defined(OS_WIN)
  // On Windows we need to specify certain flags that will be passed to
  // CreateWindowEx().
//   window_info.SetAsPopup(NULL, "cefsimple");
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = 1000;
  rect.bottom = 500;
	window_info.SetAsChild(m_hParent, rect);
#endif

  // SimpleHandler implements browser-level callbacks.
//   CefRefPtr<SimpleHandler> handler(new SimpleHandler());

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  std::string url;

  // Check if a "--url=" value was provided via the command-line. If so, use
  // that instead of the default URL.
  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();
  url = command_line->GetSwitchValue("url");
  if (url.empty())
    url = "http://www.baidu.com";

  // Create the first browser window.
  CefBrowserHost::CreateBrowser(window_info, SimpleHandler::GetInstance(), url,
                                browser_settings, NULL);

//   CefRefPtr<CefBrowser> browser = CefBrowserHost::get
}
