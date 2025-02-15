#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <thread>
#include <windows.h>
#include <filesystem>
#include "console.hpp"
#include "FontManager.hpp"
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace Utils {

	auto CALLBACK FontManager::FontListBox::EnumProc(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD fontType, LPARAM lParam) -> int {
		if (fontType == TRUETYPE_FONTTYPE && lpelfe->elfLogFont.lfFaceName[0] != 0x40) {
			if (auto&& name = std::wstring_view(lpelfe->elfLogFont.lfFaceName); !name.empty()) {
				if (m_this->defaultIndex == -1 && name == reinterpret_cast<wchar_t*>(lParam)) {
					m_this->defaultIndex = m_this->GetCount();
				}
				m_this->AddItem(lpelfe->elfLogFont.lfFaceName);
			}
		}
		return 1;
	}

	auto CALLBACK FontManager::ManagerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT {
		switch (uMsg)
		{
		case WM_PAINT: {
			m_this->InitDisplay();
			break;
		}
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORSTATIC: {
			::SetBkColor(reinterpret_cast<HDC>(wParam), m_this->DefaultBackgroundColor);
			return reinterpret_cast<LRESULT>(m_this->DefaultSolidBrush);
		}
		case WM_COMMAND: {
			
			if (LOWORD(wParam) == m_this->m_FontListBox.IDC && HIWORD(wParam) == LBN_SELCHANGE) {
				if (auto&& name = m_this->m_FontListBox.GetCurrentName(); 
					!name.empty() && name != m_this->currentData.name) 
				{
					::wcscpy_s(m_this->currentData.name, name.c_str());
					m_this->m_FszGroupBox.nameEditor.SetTextW(m_this->currentData.name);
					m_this->UpdateDisplay(true);
				}
			}
			else if (LOWORD(wParam) == m_this->m_ResetButton.IDC) {
				if (std::memcmp(&m_this->currentData, &m_this->defaultData, sizeof(FontManager::Data))) {
					m_this->currentData = m_this->defaultData;
					m_this->UpdateBoxState().UpdateDisplay(true);
					if (m_this->m_FontListBox.ResetDefault() == -1 && m_this->m_FontListBox
						.SelectItem(m_this->currentData.name) == LB_ERR) 
					{
						m_this->m_FontListBox.SendMessage(LB_SETCURSEL, -1, 0);
					}
				}
			}
			else if (LOWORD(wParam) == m_this->m_ApplyButton.IDC) {
				m_this->StorageData();
				if (auto&& name = m_this->m_FszGroupBox.nameEditor.GetTextW();
					!name.empty() && name != m_this->currentData.name)
				{
					::wcscpy_s(m_this->currentData.name, name.c_str());
					if (m_this->m_FontListBox.SelectItem(name.c_str()) == LB_ERR) {
						m_this->m_FontListBox.SendMessage(LB_SETCURSEL, -1, 0);
					}
					m_this->UpdateDisplay();
				}
				m_this->lastData = m_this->currentData;
				if (m_this->m_Callback) m_this->m_Callback(m_this);
				m_this->m_DataUpdate = false;
				m_this->SetTextW(L"字体设置");
			}

			break;
		}
		case WM_CLOSE: {
			if (m_this->m_DataUpdate) {
				if (::MessageBoxW(m_this->m_hwnd, L"是否应用当前字体样式？", L"*未应用", MB_YESNO) != IDYES) {
					m_this->currentData = m_this->lastData;
				}
				else if (m_this->m_Callback) {
					m_this->m_Callback(m_this);
				}
				m_this->UpdateBoxState().UpdateDisplay().StorageData();
				m_this->m_DataUpdate = false;
			}
			m_this->m_OnChoosing = false;
			m_this->HideWindow();
			return TRUE;
		}
		case WM_NCRBUTTONDOWN: return NULL;
		default: break;
		}

		return m_this->m_proc(hwnd, uMsg, wParam, lParam);
	}


	auto CALLBACK FontManager::FszGroupBox::Editor::EditorProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT {
		if (uMsg == WM_CHAR && wParam == 0x01) { // ctrl + a 全选文本框内容
			return ::SendMessageW(hwnd, EM_SETSEL, 0, -1);
		}
		if (uMsg == WM_CHAR && wParam == 0x0D) { // 按下回车键事件，将当前字体设置搜索结果的第一个结果
			if (auto text = m_this->GetTextW(); text.empty()) {
				m_this->SetTextW(m_this->manager->currentData.name);
				m_this->manager->UpdateDisplay(true);
			}
			else if (int index = m_this->manager->m_FontListBox.FindItem(text.c_str(), false); 
				LB_ERR != index && index != m_this->manager->m_FontListBox.GetCurrentIndex()) {
				m_this->manager->m_FontListBox.SelectItem(index);
				auto&& name = m_this->manager->m_FontListBox.GetCurrentName();
				::wcscpy_s(m_this->manager->currentData.name, 30, name.c_str());
				m_this->SetTextW(m_this->manager->currentData.name);
				m_this->manager->UpdateDisplay(true);
			}
			return TRUE;
		}
		return m_this->m_proc(hwnd, uMsg, wParam, lParam);
	}

	auto CALLBACK FontManager::FszGroupBox::BoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT {
		if (::GetDlgCtrlID(HWND(lParam)) == m_this->nameEditor.IDC && uMsg == WM_COMMAND && HIWORD(wParam) == EN_CHANGE) {
			if (auto&& text = m_this->nameEditor.GetTextW(); text.empty()) {
				m_this->manager->m_FontListBox.ResetTopIndex();
			}
			else {
				m_this->manager->m_FontListBox.FindItem(text.c_str(), false);
			}
			return TRUE;
		}
		if (::GetDlgCtrlID(HWND(lParam)) == m_this->trackBar.IDC && uMsg == WM_HSCROLL) {
			m_this->manager->currentData.size = m_this->trackBar.GetValue();
			m_this->sizeText.SetValue(m_this->manager->currentData.size);
			m_this->manager->UpdateDisplay(true);
		}

		if(uMsg == WM_CTLCOLORSTATIC ) {
			::SetBkColor(reinterpret_cast<HDC>(wParam), m_this->manager->DefaultBackgroundColor);
			return reinterpret_cast<LRESULT>(m_this->manager->DefaultSolidBrush);
		}
		return m_this->m_proc(hwnd, uMsg, wParam, lParam);
	}
	
	auto CALLBACK FontManager::FontListBox::BoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT {

		return m_this->m_proc(hwnd, uMsg, wParam, lParam);
	}

	auto CALLBACK FontManager::StyGroupBox::BoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT {
		if (uMsg == WM_COMMAND && HIWORD(wParam) == BN_CLICKED) {
			if (m_this->manager->currentData.style != Style(LOWORD(wParam))) {
				m_this->manager->currentData.style = Style(LOWORD(wParam));
				m_this->manager->UpdateDisplay(true);
			}
			return TRUE;
		}
		if(uMsg == WM_CTLCOLORSTATIC) {
			::SetBkColor(reinterpret_cast<HDC>(wParam), m_this->manager->DefaultBackgroundColor);
			return reinterpret_cast<LRESULT>(m_this->manager->DefaultSolidBrush);
		}
		return m_this->m_proc(hwnd, uMsg, wParam, lParam);
	}

	inline FontManager::FontListBox::FontListBox(FontManager* manager, HFONT font, HINSTANCE hInstance) : WindowBase(WS_EX_LTRREADING, WC_LISTBOX, WS_VISIBLE |
		WS_CHILD | WS_VSCROLL | WS_BORDER | LBS_HASSTRINGS | LBS_NOTIFY, 15, 85, 275, 240, manager->m_hwnd, hInstance, NULL), manager(manager)
	{
		this->SetFont(font).SetProc(FontListBox::BoxProc);
	}

	inline auto FontManager::FontListBox::Init(const wchar_t* name, LOGFONT logfont) const -> const FontManager::FontListBox& {
		::EnumFontFamiliesExW(this->GetDC(), &logfont, FONTENUMPROCW(FontListBox::EnumProc), LPARAM(name), NULL);
		return *this;
	}

	inline auto FontManager::FontListBox::Init(const wchar_t* name) const -> const FontManager::FontListBox& {
		return this->Init(name, { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, ANSI_CHARSET });
	}

	inline auto FontManager::FontListBox::AddItem(const wchar_t* item) const -> int {
		return this->SendMessage(LB_ADDSTRING, 0, LPARAM(item));
	}

	inline auto FontManager::FontListBox::GetCount() const -> int {
		return this->SendMessage(LB_GETCOUNT, 0, 0);
	}

	inline auto FontManager::FontListBox::SelectItem(int index) const -> int {
		if (index == -1) return LB_ERR;
		return this->SendMessage(LB_SETCURSEL, index, 0);
	}

	inline auto FontManager::FontListBox::SelectItem(const wchar_t* name, bool exact) const -> int {
		if (auto&& index = this->FindItem(name, exact, false); LB_ERR != index) {
			return this->SelectItem(index);
		}
		return LB_ERR;
	}

	inline auto FontManager::FontListBox::FindItem(const wchar_t* name, bool exact, bool redraw) const -> int {
		if (auto&& index = this->SendMessage(exact ? LB_FINDSTRINGEXACT : LB_FINDSTRING, -1, LPARAM(name)); index != LB_ERR) {
			if (redraw) this->SetTopIndex(index);
			return index;
		}
		return LB_ERR;
	}

	inline auto FontManager::FontListBox::GetTopIndex() const -> int {
		return this->SendMessage(LB_GETTOPINDEX, 0, 0);
	}

	inline auto FontManager::FontListBox::SetTopIndex(int index) const -> int {
		return this->SendMessage(LB_SETTOPINDEX, index, 0), this->GetTopIndex();
	}

	inline auto FontManager::FontListBox::ResetTopIndex() const -> int {
		return this->SetTopIndex(this->GetCurrentIndex());
	}

	inline auto FontManager::FontListBox::ResetDefault() const -> int {
		if (this->defaultIndex != -1) {
			this->SelectItem(this->defaultIndex);
		}
		return this->defaultIndex;
	}

	inline auto FontManager::FontListBox::GetCurrentIndex() const -> int {
		return this->SendMessage(LB_GETCURSEL, 0, 0);
	}

	inline auto FontManager::FontListBox::GetCurrentName() const -> std::wstring {
		if (const auto&& index = this->GetCurrentIndex(); this->SendMessage(LB_GETTEXTLEN, index, 0) > 0) {
			if (wchar_t buffer[32]{}; LB_ERR != this->SendMessage(LB_GETTEXT, index, LPARAM(buffer))) {
				return std::wstring(buffer);
			}
		}
		return std::wstring(L"");
	}

	inline FontManager::FszGroupBox::FszGroupBox(FontManager* manager, HFONT font, HINSTANCE hInstance) : WindowBase(
		WS_EX_LTRREADING, L"BUTTON", L"字体＆大小", WS_VISIBLE | WS_CHILD | BS_GROUPBOX | BS_CENTER , 305, 75, 220, 115,
		manager->m_hwnd, hInstance), nameEditor(manager, this->m_hwnd, font, hInstance), trackBar(this->m_hwnd, hInstance),
		sizeText(this->m_hwnd, font, hInstance), manager(manager)
	{
		this->SetFont(font).SetProc(FszGroupBox::BoxProc);
		this->nameEditor.SetLimitText(30);
	}

	inline FontManager::FszGroupBox::Editor::Editor(FontManager* manager, HWND parent, HFONT font, HINSTANCE hInstance): WindowBase(WS_EX_CLIENTEDGE,
		L"EDIT", WS_VISIBLE | WS_CHILD | ES_MULTILINE, 10, 30, 200, 30, parent, hInstance), manager(manager)
	{
		this->SetFont(font).SetProc(Editor::EditorProc);
	}

	inline auto FontManager::FszGroupBox::Editor::SetLimitText(int limit) const -> void {
		this->SendMessage(EM_LIMITTEXT, limit, NULL);
	}

	inline auto FontManager::FszGroupBox::Editor::SelectAllText() const -> void {
		this->SendMessage(EM_SETSEL, 0, -1);
	}

	inline FontManager::FszGroupBox::Trackbar::Trackbar(HWND parent, HINSTANCE hInstance): WindowBase(WS_EX_LTRREADING,
		TRACKBAR_CLASS, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, 10, 75, 170, 30, parent, hInstance) {
	}

	inline auto FontManager::FszGroupBox::Trackbar::SetRange(int min, int max, bool redraw) const -> const Trackbar& {
		return this->SendMessage(TBM_SETRANGE, redraw, MAKELPARAM(min, max)), * this;
	}
	
	inline auto FontManager::FszGroupBox::Trackbar::SetValue(int value, bool redraw) const -> const Trackbar& {
		return this->SendMessage(TBM_SETPOS, redraw, value), *this;
	}

	inline auto Utils::FontManager::FszGroupBox::Trackbar::GetValue() const -> int {
		return this->SendMessage(TBM_GETPOS, 0, 0);
	}

	inline auto FontManager::FszGroupBox::Trackbar::Set(int min, int max, int value, bool redraw) const -> const Trackbar& {
		return this->SetRange(min, max).SetValue(value, redraw);
	}

	inline FontManager::FszGroupBox::SizeText::SizeText(HWND parent, HFONT font, HINSTANCE hInstance) : WindowBase(
		WS_EX_LTRREADING, L"STATIC", WS_VISIBLE | WS_CHILD, 190, 80, 20, 20, parent, hInstance) {
		this->SetFont(font);
	}

	inline auto FontManager::FszGroupBox::SizeText::SetValue(int value) const -> bool {
		return this->SetTextW(std::to_wstring(value).c_str());
	}

	inline FontManager::StyGroupBox::StyGroupBox(FontManager* manager, HFONT font, HINSTANCE hInstance): WindowBase(
		WS_EX_LTRREADING, L"BUTTON", L"字体样式", WS_VISIBLE | WS_CHILD | BS_GROUPBOX | BS_CENTER,
		305, 195, 220, 75, manager->m_hwnd, hInstance), button1(this->m_hwnd, font, hInstance),
		button2(this->m_hwnd, font, hInstance), button3(this->m_hwnd, font, hInstance),
		button4(this->m_hwnd, font, hInstance), manager(manager)
	{
		this->SetFont(font).SetProc(StyGroupBox::BoxProc);
	}

	auto FontManager::StyGroupBox::SetChecked(uint16_t btn) const -> bool {
		switch (btn)
		{
		case StyGroupBox::Button1::IDC:
			this->button1.Checked();
			this->button2.UnChecked();
			this->button3.UnChecked();
			this->button4.UnChecked();
			return true;
		case StyGroupBox::Button2::IDC:
			this->button1.UnChecked();
			this->button2.Checked();
			this->button3.UnChecked();
			this->button4.UnChecked();
			return true;
		case StyGroupBox::Button3::IDC:
			this->button1.UnChecked();
			this->button2.UnChecked();
			this->button3.Checked();
			this->button4.UnChecked();
			return true;
		case StyGroupBox::Button4::IDC:
			this->button1.UnChecked();
			this->button2.UnChecked();
			this->button3.UnChecked();
			this->button4.Checked();
			return true;
		default: return false;
		}
	}

	inline auto FontManager::StyGroupBox::GetChecked() const -> uint16_t {
		if (this->button1.IsChecked()) {
			return this->button1.IDC;
		}
		if (this->button2.IsChecked()) {
			return this->button2.IDC;
		}
		if (this->button3.IsChecked()) {
			return this->button3.IDC;
		}
		if (this->button4.IsChecked()) {
			return this->button4.IDC;
		}
		return NULL;
	}
	
	FontManager::ActCtxHelper::ActCtxHelper(HMODULE dllInstance) {
		this->Init(dllInstance);
	}

	FontManager::ActCtxHelper::~ActCtxHelper() {
		if (this->m_ActCtx) ::ReleaseActCtx(this->m_ActCtx);
		this->m_ActCtx = { INVALID_HANDLE_VALUE };
	}

	auto FontManager::ActCtxHelper::Init(HMODULE dllInstance, ACTCTX actCtx) -> void {
		if (this->m_ActCtx) ::ReleaseActCtx(this->m_ActCtx);
		actCtx.cbSize = sizeof(actCtx);
		actCtx.hModule = dllInstance;
		actCtx.lpResourceName = MAKEINTRESOURCE(2);
		actCtx.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;
		this->m_ActCtx = CreateActCtxW(&actCtx);
	}

	auto FontManager::ActCtxHelper::Get() const -> HANDLE {
		return this->m_ActCtx;
	}

	auto FontManager::ActCtxHelper::Activate() -> bool {
		if (this->m_ActCtx != INVALID_HANDLE_VALUE) {
			return ::ActivateActCtx(this->m_ActCtx, &this->m_Cookie);
		}
		return false;
	}

	auto FontManager::ActCtxHelper::Deactivate() -> bool {
		bool result{ false };
		if (this->m_ActCtx != INVALID_HANDLE_VALUE && this->m_Cookie) {
			result = ::DeactivateActCtx(0, this->m_Cookie);
		}
		this->m_Cookie = {};
		return result;
	}

	auto FontManager::Init(INITCOMMONCONTROLSEX icc, WNDCLASSEX cls) -> void {
		::InitCommonControlsEx(&icc);
		cls.lpfnWndProc = ::DefWindowProcW;
		cls.lpszClassName = FontManager::ManagerClassName;
		::RegisterClassExW(&cls);
	}

	auto FontManager::InitVisStyActCtx(HMODULE dllInstance) -> void {
		FontManager::VisStyActCtx->Init(dllInstance);
	}

	auto FontManager::IsFullScreen() const -> bool {
		int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
		LONG lStyle = this->m_Parent.Get(GWL_STYLE);
		RECT rect = this->m_Parent.GetRect();
		return (lStyle & WS_POPUP) == WS_POPUP &&
			rect.left <= 0 && rect.top <= 0 &&
			rect.right >= nScreenWidth &&
			rect.bottom >= nScreenHeight;
	}

	auto FontManager::CreatePtr(HWND parent, HFONT hFont, HINSTANCE hInstance) -> std::unique_ptr<FontManager> {
		FontManager::Init(); 
		FontManager::VisStyActCtx->Activate();
		auto result = std::unique_ptr<FontManager>(new FontManager(parent, hFont, hInstance));
		FontManager::VisStyActCtx->Deactivate();
		return result;
	}

	auto FontManager::CreatePtr(HFONT hFont, HINSTANCE hInstance) -> std::unique_ptr<FontManager> {
		return FontManager::CreatePtr(NULL, hFont, hInstance);
	}

	auto FontManager::CreatePtr(HWND parent, HINSTANCE hInstance) -> std::unique_ptr<FontManager> {
		return FontManager::CreatePtr(parent, ::CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE,
			FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, L"微软雅黑"), hInstance);
	}

	auto FontManager::Init(Data defaultData, int minSize, int maxSize) -> FontManager& {
		this->m_FszGroupBox.trackBar.SetRange(minSize, maxSize, false);
		this->m_FontListBox.Init(defaultData.name);
		return this->defaultData = defaultData, *this;
	}

	auto FontManager::Init(int size, Style style, std::wstring_view name, int minSize, int maxSize) -> FontManager& {
		this->defaultData = Data{ int16_t(size), style };
		this->m_FszGroupBox.trackBar.SetRange(minSize, maxSize, false);
		if (auto length = name.length(); length >0 && length < 32) {
			::wcscpy_s(this->defaultData.name, name.data());
			this->m_FontListBox.Init(name.data());
		}
		return *this;
	}

	auto FontManager::Load(Data currentData) -> FontManager& {
		return this->Load(currentData.size, currentData.style, currentData.name);
	}

	auto FontManager::Load(int size, Style style, std::wstring_view name) -> FontManager& {
		this->currentData = Data{ int16_t(size), style };
		if (auto length = name.length(); length > 0 && length < 32) {
			::wcscpy_s(this->currentData.name, name.data());
		}
		this->m_FszGroupBox.trackBar.SetValue(currentData.size);
		this->m_FszGroupBox.sizeText.SetValue(currentData.size);
		this->m_FszGroupBox.nameEditor.SetTextW(currentData.name);
		this->m_FontListBox.SelectItem(currentData.name);
		this->m_StyGroupBox.SetChecked(currentData.style);
		return *this;
	}

	auto FontManager::Load(std::string_view storageFilePath, bool init) -> FontManager& {
		if (!storageFilePath.empty()) {
			if (FILE* file = std::fopen(storageFilePath.data(), "rb")) {
				std::fseek(file, 0, SEEK_END);
				if (std::ftell(file) >= sizeof(Data)) {
					std::rewind(file);
					std::fread(&this->currentData, sizeof(Data), 1, file);
					std::fclose(file);
				}
			}
			else if(init) {
				this->currentData = this->defaultData;
				this->StorageData(storageFilePath);
			}
			this->m_storageFilePath.assign(storageFilePath);
			this->UpdateBoxState();
		}
		return *this;
	}

	auto FontManager::StorageData(std::string_view storageFilePath) const -> bool {
		if (storageFilePath.empty()) return false;
		if (auto&& dir = std::filesystem::path(storageFilePath).parent_path(); !dir.string().empty()) {
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
		}
		if (FILE* file = std::fopen(storageFilePath.data(), "wb")) {
			size_t result = std::fwrite(&this->currentData, sizeof(Data), 1, file);
			std::fclose(file);
			return result == 1;
		}
		return false;
	}

	auto FontManager::StorageData() const -> bool {
		return this->StorageData(this->m_storageFilePath.c_str());
	}

	auto FontManager::OnChanged(Callback callback) -> FontManager& {
		this->m_Callback = callback;
		return *this;
	}
	auto FontManager::OnChanged(std::function<void(int size, Style style, const std::wstring_view name)> callback) -> FontManager& {
		return this->OnChanged([callback](const FontManager* m_this) -> void {
			callback(m_this->currentData.size, m_this->currentData.style, m_this->currentData.name);
		});
	}

	auto FontManager::GetData() const -> const Data & {
		return this->currentData;
	}

	auto FontManager::ShowWindow(bool topMost) const-> BOOL {
		int x{ CW_USEDEFAULT }, y{ CW_USEDEFAULT }, width{ 550 }, height{ 355 };
		if (RECT windowRect{}; ::GetWindowRect(this->Get<HWND>(GWLP_USERDATA), &windowRect) ||
			::GetWindowRect(GetDesktopWindow(), &windowRect)) {
			x = ((windowRect.left + windowRect.right) / 2) - (width / 2);
			y = ((windowRect.top + windowRect.bottom) / 2) - (height / 2);
		}
		::SetWindowPos(this->m_hwnd, topMost ? HWND_TOPMOST: HWND_NOTOPMOST, x, y, NULL, NULL, SWP_NOSIZE | SWP_NOACTIVATE);
		::SetForegroundWindow(this->m_hwnd);
		return ::ShowWindow(this->m_hwnd, SW_SHOW);
	}

	auto FontManager::HideWindow() const-> BOOL {
		return ::ShowWindow(m_this->m_hwnd, SW_HIDE);
	}

	auto FontManager::MessageLoop() const -> void {
		std::thread([](HWND hwnd,MSG msg = { NULL }) {
			while (::GetMessageW(&msg, hwnd, 0, 0) > 0) {
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
		}, this->m_hwnd).detach();
	}

	auto FontManager::ChooseFont() -> FontManager& {
		this->lastData = this->currentData;
		this->m_OnChoosing = true;
		this->ShowWindow(this->IsFullScreen());
		return *this;
	}

	auto Utils::FontManager::ChooseFont(std::function<void(int size, Style style, const std::wstring_view name)> callback) -> FontManager& {
		std::thread([this, callback]() {
			this->ChooseFont().Wait();
			callback(this->currentData.size, this->currentData.style, this->currentData.name);
		}).detach();
		return *this;
	}

	auto FontManager::Wait() -> FontManager& {
		while (this->m_OnChoosing);
		return *this;
	}

	auto Utils::FontManager::MakeFont(DWORD iCharSet) const -> HFONT {
		auto&& name = ::wcslen(this->currentData.name) > 0 ? this->currentData.name : 
			::wcslen(this->defaultData.name) > 0 ? this->defaultData.name : L"";
		auto&& size = this->currentData.size > 0 ? this->currentData.size : this->defaultData.size;
		auto&& style = this->currentData.style ? this->currentData.style : this->defaultData.style;
		return ::CreateFontW(size, 0, 0, 0, style & static_cast<uint16_t>(0x00F0) ? FW_BOLD : FW_NORMAL,
			style & static_cast<uint16_t>(0x0F00) ? TRUE : FALSE, FALSE, FALSE, iCharSet, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, name);
	}

	auto Utils::FontManager::MakeDefualtFont(DWORD iCharSet) const -> HFONT {
		return ::CreateFontW(this->defaultData.size, 0, 0, 0, this->defaultData.style & static_cast<uint16_t>(0x00F0) ? FW_BOLD : FW_NORMAL,
			this->defaultData.style & static_cast<uint16_t>(0x0F00) ? TRUE : FALSE, FALSE, FALSE, iCharSet, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, this->defaultData.name);
	}


	FontManager::FontManager(HWND parent, HFONT font, HINSTANCE hInstance) : WindowBase(
		WS_EX_LTRREADING, FontManager::ManagerClassName, L"字体设置", WS_SYSMENU | WS_CAPTION, NULL,
		NULL, 550, 375, NULL, NULL, NULL, hInstance), m_hFont(font), m_StyGroupBox(this, font, hInstance),
		m_ApplyButton(this->m_hwnd, font, hInstance), m_ResetButton(this->m_hwnd, font, hInstance),
		m_FszGroupBox(this, font, hInstance), m_FontListBox(this, font, hInstance), m_Parent(parent)
	{
		this->Set(GWLP_USERDATA, parent);
		this->SetFont(font).SetProc(FontManager::ManagerWndProc);
		this->SetIcon(reinterpret_cast<HICON>(::GetClassLongW(parent, GCLP_HICON)));
	}

	auto FontManager::InitDisplay(SIZE size, PAINTSTRUCT ps) -> FontManager& {
		static constexpr wchar_t text[]{ L"※请适当调整字体大小，过大过小都可能会导致游戏内显示异常。" };
		HDC hdc = ::BeginPaint(this->m_hwnd, &ps);
		::FillRect(hdc, &ps.rcPaint, this->DefaultSolidBrush);
		::SelectObject(hdc, this->m_hFont);
		::SetTextColor(hdc, RGB(193, 0, 0));
		::GetTextExtentPoint32W(hdc, text, (sizeof(text) - 2) / 2, &size);
		::SetBkMode(hdc, TRANSPARENT);
		::TextOutW(hdc, ((550 - size.cx) / 2), 316, text, (sizeof(text) - 2) / 2);
		::EndPaint(this->m_hwnd, &ps);
		return this->UpdateDisplay();
	}

	auto FontManager::UpdateDisplay(bool state, SIZE size, PAINTSTRUCT ps) -> FontManager& {
		static const RECT rect{ 0, 0, 550, 70 };
		static constexpr wchar_t text[]{ L"这是一段测试字体样式的文字。" };
		this->SetTextW(state ? L"字体设置 *未应用" : L"字体设置");
		::InvalidateRect(this->m_hwnd, &rect, TRUE);
		HDC hdc = ::BeginPaint(this->m_hwnd, &ps);
		::FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
		const auto&& hFont = this->MakeFont();
		::SelectObject(hdc, hFont);
		::SetTextColor(hdc, RGB(0, 0, 0));
		::SetTextCharacterExtra(hdc, 2);
		::GetTextExtentPoint32W(hdc, text, (sizeof(text) - 2) / 2, &size);
		::TextOutW(hdc, ((550 - size.cx) / 2), ((70 - size.cy) / 2), text, (sizeof(text) - 2) / 2);
		::EndPaint(this->m_hwnd, &ps);
		::DeleteObject(hFont);
		this->m_DataUpdate = state;
		return *this;
	}

	auto Utils::FontManager::UpdateBoxState() -> FontManager& {
		this->m_FszGroupBox.nameEditor.SetTextW(this->currentData.name);
		this->m_FszGroupBox.trackBar.SetValue(this->currentData.size);
		this->m_FszGroupBox.sizeText.SetValue(this->currentData.size);
		this->m_StyGroupBox.SetChecked(this->currentData.style);
		this->m_FontListBox.SelectItem(this->currentData.name);
		return *this;
	}

	auto FontManager::IsWindowVisible() -> bool {
		return static_cast<bool>(::IsWindowVisible(this->m_hwnd));
	}

	auto FontManager::UpdateDisplayState() -> FontManager& {
		if (this->IsWindowVisible()) {
			this->ShowWindow(this->IsFullScreen());
		}
		return *this;
	}
}