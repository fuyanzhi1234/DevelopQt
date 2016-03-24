// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"
#include <list>
#include <QObject>
#include <QString>

class SimpleHandler : public QObject,
	public CefClient,
	public CefContextMenuHandler,
	public CefDisplayHandler,
	public CefLifeSpanHandler,
	public CefLoadHandler,
	public CefRequestHandler,
	public CefRenderHandler {

	Q_OBJECT

public:
	SimpleHandler();
	~SimpleHandler();

	// Provide access to the single global instance of this object.
	static SimpleHandler* GetInstance();

	// CefClient methods
// 	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE{
// 		return this;
// 	}
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE{
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE{
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE{
		return this;
	}
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE{
		return this;
	}
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() OVERRIDE{
		return this;
	}

	// CefDisplayHandler methods:
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
		const CefString& title) OVERRIDE;

	// CefLifeSpanHandler methods:
	virtual bool OnBeforePopup(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		CefLifeSpanHandler::WindowOpenDisposition target_disposition,
		bool user_gesture,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		bool* no_javascript_access) OVERRIDE;
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

	// CefLoadHandler methods:
	virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
		bool isLoading,
		bool canGoBack,
		bool canGoForward) OVERRIDE;

	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame) OVERRIDE;

	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		int httpStatusCode) OVERRIDE;

	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl) OVERRIDE;

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);

	bool IsClosing() const { return is_closing_; }

	// CefContextMenuHandler methods
	void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params,
		CefRefPtr<CefMenuModel> model) OVERRIDE;
	bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params,
		int command_id,
		EventFlags event_flags) OVERRIDE;

	// CefRequestHandler methods
	bool OnCertificateError(
		CefRefPtr<CefBrowser> browser,
		ErrorCode cert_error,
		const CefString& request_url,
		CefRefPtr<CefSSLInfo> ssl_info,
		CefRefPtr<CefRequestCallback> callback) OVERRIDE;

	bool GetAuthCredentials(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		bool isProxy,
		const CefString& host,
		int port,
		const CefString& realm,
		const CefString& scheme,
		CefRefPtr<CefAuthCallback> callback);

	void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
		TerminationStatus status) OVERRIDE;

	// CefRenderHandler methods
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) { return false; }

	virtual void OnPaint(CefRefPtr<CefBrowser> browser,
		PaintElementType type,
		const RectList& dirtyRects,
		const void* buffer,
		int width, int height) {};

	virtual void OnCursorChange(CefRefPtr<CefBrowser> browser,
		CefCursorHandle cursor,
		CursorType type,
		const CefCursorInfo& custom_cursor_info) OVERRIDE;
private:

signals :
	// ��ʾ��Ȩ�Ի���
	void showAuthorityDialog(int browserIdentifier, QString userName, QString userPassword);

		// �����ɹ���֪ͨ������
		void creatBrowserSuccess(HWND parentWnd, HWND browserWnd, int browserIdentifier);

		// ��ʼ����
		void loadStart(int browserIdentifier);

		// ��������
		void loadEnd(int browserIdentifier, int httpStatusCode);

public:
	// ���ָ�������
	CefRefPtr<CefBrowser> GetBrowser(int browserIdentifier);
	// ������ʾҳ��
	void SetZoom(int browserIdentifier, double delta);
	// ˢ�������
	void Refresh(int browserIdentifier);
	// ��Ȩ�û������룬��ˢ��
	void RefreshWithAuthInfo(int browserIdentifier, QString userName, QString userPassword);

	/*
	����������ǰ���ʵ�url
	@Param int browserIdentifier : �������ʶ
	@Return : QString
	*/
	QString getCurrentURL(int browserIdentifier);

	/*
	ֹͣ������ҳ
	@Param int browserIdentifier : �������ʶ
	@Return void :
	*/
	void stopLoad(int browserIdentifier);

	/*
	������ҳ
	@Param int browserIdentifier : �������ʶ
	@Param QString url : ��վ��ַ
	@Return void :
	*/
	void loadURL(int browserIdentifier, QString url);

	/*
	��ʾdevtool
	@Param int browserIdentifier : �������ʶ
	@Return void : 
	*/
	void showDevTool(int browserIdentifier);

private:
	// single instance
	static SimpleHandler *instance;
	// List of existing browser windows. Only accessed on the CEF UI thread.
	typedef std::list<CefRefPtr<CefBrowser> > BrowserList;
	BrowserList browser_list_;

	bool is_closing_;

	QString m_userName;
	QString m_userPassword;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
