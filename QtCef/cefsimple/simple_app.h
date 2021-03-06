// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "include/cef_app.h"

class SimpleApp : public CefApp,
	public CefBrowserProcessHandler{
public:
	SimpleApp(HWND hParent = NULL);

	// CefApp methods:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
		OVERRIDE{ return this; }


	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() OVERRIDE;

	// �����д���
	virtual void OnBeforeCommandLineProcessing(
		const CefString& process_type,
		CefRefPtr<CefCommandLine> command_line);

private:
	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SimpleApp);

protected:
	HWND m_hParent;
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
