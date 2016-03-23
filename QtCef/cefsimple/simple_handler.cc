// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include <QMessageBox>
#include <QDebug>

namespace {

	void ModifyZoom(CefRefPtr<CefBrowser> browser, double delta) {
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute on the UI thread.
			CefPostTask(TID_UI, base::Bind(&ModifyZoom, browser, delta));
			return;
		}

		browser->GetHost()->SetZoomLevel(
			browser->GetHost()->GetZoomLevel() + delta);
	}
}  // namespace

SimpleHandler *SimpleHandler::instance = NULL;
SimpleHandler::SimpleHandler()
: is_closing_(false) {
}

SimpleHandler::~SimpleHandler() {
	instance = NULL;
}

// static
SimpleHandler* SimpleHandler::GetInstance() {
	if (instance == NULL)
	{
		instance = new SimpleHandler();
	}

	return instance;
}

// CefLifeSpanHandler methods:
bool SimpleHandler::OnBeforePopup(
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
	bool* no_javascript_access) {
	CEF_REQUIRE_IO_THREAD();

	browser->GetMainFrame()->LoadURL(target_url);

	// Return true to cancel the popup window.
	return true;
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// 可以控制鼠标形状是否可以改变
	//   browser->GetHost()->SetMouseCursorChangeDisabled(true);
	// Add to the list of existing browsers.
	int browserIdentifier = browser->GetIdentifier();
	CefWindowHandle browserHwnd = browser->GetHost()->GetWindowHandle();
	HWND parentHwnd = GetParent(browserHwnd);
	emit creatBrowserSuccess(parentHwnd, browserHwnd, browserIdentifier);

	// 将browser保存起来
	browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed destription of this
	// process.
	if (browser_list_.size() == 1) {
		// Set a flag to indicate that the window close should be allowed.
		is_closing_ = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();

	// Remove from the list of existing browsers.
	BrowserList::iterator bit = browser_list_.begin();
	for (; bit != browser_list_.end(); ++bit) {
		if ((*bit)->IsSame(browser)) {
			browser_list_.erase(bit);
			break;
		}
	}

	if (browser_list_.empty()) {
		// All browser windows have closed. Quit the application message loop.
		//     CefQuitMessageLoop();
	}
}

void SimpleHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
	bool isLoading,
	bool canGoBack,
	bool canGoForward)
{
	qDebug() << "OnLoadingStateChange : " << isLoading << ", " << canGoBack << ", " << canGoForward;
}

void SimpleHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame)
{
	if (frame->IsMain())
	{
		emit loadStart(browser->GetIdentifier());
	}
}

void SimpleHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	int httpStatusCode)
{
	if (frame->IsMain())
	{
		emit loadEnd(browser->GetIdentifier(), httpStatusCode);
	}
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();

	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;

	// Display a load error message.
	std::stringstream ss;
	ss << "<html><body bgcolor=\"white\">"
		"<h2>Failed to load URL " << std::string(failedUrl) <<
		" with error " << std::string(errorText) << " (" << errorCode <<
		").</h2></body></html>";
	frame->LoadString(ss.str(), failedUrl);
}

void SimpleHandler::CloseAllBrowsers(bool force_close) {
	if (!CefCurrentlyOn(TID_UI)) {
		// Execute on the UI thread.
		CefPostTask(TID_UI,
			base::Bind(&SimpleHandler::CloseAllBrowsers, this, force_close));
		return;
	}

	if (browser_list_.empty())
		return;

	BrowserList::const_iterator it = browser_list_.begin();
	for (; it != browser_list_.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);
}

// CefContextMenuHandler methods
void SimpleHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	CefRefPtr<CefMenuModel> model)
{
	if (model)
	{
		// 可以屏蔽鼠标右键
// 		model->Clear();
	}
}

bool SimpleHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	int command_id,
	EventFlags event_flags)
{
	return false;
}

bool SimpleHandler::OnCertificateError(
	CefRefPtr<CefBrowser> browser,
	ErrorCode cert_error,
	const CefString& request_url,
	CefRefPtr<CefSSLInfo> ssl_info,
	CefRefPtr<CefRequestCallback> callback) {
	CEF_REQUIRE_UI_THREAD();

	CefRefPtr<CefSSLCertPrincipal> subject = ssl_info->GetSubject();
	CefRefPtr<CefSSLCertPrincipal> issuer = ssl_info->GetIssuer();

	return false;  // Cancel the request.
}

// 显示授权窗口
bool SimpleHandler::GetAuthCredentials(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	bool isProxy,
	const CefString& host,
	int port,
	const CefString& realm,
	const CefString& scheme,
	CefRefPtr<CefAuthCallback> callback) {
	// 因Qt线程问题，无法实现同步弹出模态窗口，所以做成异步实现
	if (m_userName.isEmpty())
	{
		emit showAuthorityDialog(browser->GetIdentifier(), m_userName, m_userPassword);
		return false;
	}

	callback->Continue(m_userName.toStdWString(), m_userPassword.toStdWString());
	// 返回用户名密码后，清空数据
	m_userName = QString();
	m_userPassword = QString();
	return true;
}

// 只有在多进程模式下，此函数才会响应
void SimpleHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
	TerminationStatus status) {
	CEF_REQUIRE_UI_THREAD();

	// 	message_router_->OnRenderProcessTerminated(browser);

	// Don't reload if there's no start URL, or if the crash URL was specified.
	// 	if (startup_url_.empty() || startup_url_ == "chrome://crash")
	// 		return;

	CefRefPtr<CefFrame> frame = browser->GetMainFrame();
	std::string url = frame->GetURL();

	// Don't reload if the termination occurred before any URL had successfully
	// loaded.
	if (url.empty())
		return;

	// 	std::string start_url = startup_url_;

	// Convert URLs to lowercase for easier comparison.
	// 	std::transform(url.begin(), url.end(), url.begin(), tolower);
	// 	std::transform(start_url.begin(), start_url.end(), start_url.begin(),
	// 		tolower);

	// Don't reload the URL that just resulted in termination.
	// 	if (url.find(start_url) == 0)
	// 		return;

	frame->LoadURL("www.baidu.com");
}

// CefRenderHandler methods
// 鼠标改变
void SimpleHandler::OnCursorChange(CefRefPtr<CefBrowser> browser,
	CefCursorHandle cursor,
	CursorType type,
	const CefCursorInfo& custom_cursor_info)
{
	qDebug() << "OnCursorChange :: CursorType : " << type;
}

// 获得指定浏览器
CefRefPtr<CefBrowser> SimpleHandler::GetBrowser(int browserIdentifier)
{
	BrowserList::const_iterator it = browser_list_.begin();
	for (; it != browser_list_.end(); ++it)
	{
		CefRefPtr<CefBrowser> browser = (*it);
		if (browser->GetIdentifier() == browserIdentifier)
		{
			return browser;
		}
	}
	return NULL;
}

// 缩放显示页面
void SimpleHandler::SetZoom(int browserIdentifier, double delta)
{
	CefRefPtr<CefBrowser> browser = GetBrowser(browserIdentifier);
	if (browser.get())
	{
		ModifyZoom(browser, delta);
	}
}

// 刷新浏览器
void SimpleHandler::Refresh(int browserIdentifier)
{
	CefRefPtr<CefBrowser> browser = GetBrowser(browserIdentifier);
	if (browser.get())
	{
		browser->Reload();
	}
}

// 授权用户名密码，并刷新
void SimpleHandler::RefreshWithAuthInfo(int browserIdentifier, QString userName, QString userPassword)
{
	m_userName = userName;
	m_userPassword = userPassword;
	Refresh(browserIdentifier);
}

QString SimpleHandler::getCurrentURL(int browserIdentifier)
{
	CefRefPtr<CefBrowser> browser = GetBrowser(browserIdentifier);
	std::string curUrl = "";
	if (browser.get())
	{
		curUrl = browser->GetMainFrame()->GetURL();
	}
	return QString::fromStdString(curUrl);
}

void SimpleHandler::stopLoad(int browserIdentifier)
{
	CefRefPtr<CefBrowser> browser = GetBrowser(browserIdentifier);
	if (browser.get())
	{
		if (browser->IsLoading())
		{
			browser->StopLoad();
		}
	}
}

void SimpleHandler::loadURL(int browserIdentifier, QString url)
{
	CefRefPtr<CefBrowser> browser = GetBrowser(browserIdentifier);
	if (browser.get())
	{
		browser->GetMainFrame()->LoadURL(url.toStdString());
	}
}
