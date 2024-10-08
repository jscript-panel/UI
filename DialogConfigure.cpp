#include "stdafx.hpp"
#include "DialogConfigure.hpp"

namespace
{
	static const CDialogResizeHelper::Param resize_data[] =
	{
		{ IDC_SCINTILLA, 0, 0, 1, 1 },
		{ IDC_BTN_TOOLS, 0, 1, 0, 1 },
		{ IDC_BTN_STYLE, 0, 1, 0, 1 },
		{ IDC_BTN_SAMPLES, 0, 1, 0, 1 },
		{ IDOK, 1, 1, 1, 1 },
		{ IDCANCEL, 1, 1, 1, 1 },
		{ IDC_BTN_APPLY, 1, 1, 1, 1 },
	};

	static const CRect resize_min_max(620, 400, 0, 0);
}

CDialogConfigure::CDialogConfigure(PanelBase* panel) : m_panel(panel), m_resizer(resize_data, resize_min_max) {}

BOOL CDialogConfigure::OnInitDialog(CWindow, LPARAM)
{
	InitTitle();
	InitScintilla();
	InitControls();
	InitFolders();

	Component::dialog_position.apply_to_window(*this);
	return TRUE;
}

LRESULT CDialogConfigure::OnNotify(int, LPNMHDR pnmh)
{
	const auto code = static_cast<Notification>(pnmh->code);

	switch (code)
	{
	case Notification::SavePointLeft:
		pfc::setWindowText(m_hWnd, fmt::format("*{}", m_title).c_str());
		break;
	case Notification::SavePointReached:
		pfc::setWindowText(m_hWnd, m_title.c_str());
		break;
	}

	SetMsgHandled(FALSE);
	return 0;
}

void CDialogConfigure::InitControls()
{
	// not a proper instance if spawned from CUI layout tab, disable Apply button
	if (!m_panel->GetWnd().IsWindow())
	{
		GetDlgItem(IDC_BTN_APPLY).EnableWindow(FALSE);
	}

	m_hooks.AddDialogWithControls(*this);
}

void CDialogConfigure::InitFolders()
{
	static const std::wstring component_folder = Path::component();
	m_licenses_folder = component_folder + L"licenses\\";
	m_samples = FileHelper(component_folder + L"samples\\").list_files();
	m_basic = FileHelper(component_folder + L"samples\\basic\\").list_files();
}

void CDialogConfigure::InitScintilla()
{
	const auto mode = g_scintilla_config.get_mode();

	m_scintilla.SubclassWindow(GetDlgItem(IDC_SCINTILLA));
	m_scintilla.Init(mode);
	m_scintilla.SetCode(m_panel->m_config.m_code);
}

void CDialogConfigure::InitTitle()
{
	SetIcon(ui_control::get()->get_main_icon());

	m_title = fmt::format("{} Configuration (id:{})", Component::name, m_panel->m_id);
	pfc::setWindowText(m_hWnd, m_title.c_str());
}

void CDialogConfigure::OnApplyOrOK(uint32_t, int nID, CWindow)
{
	Component::dialog_position.read_from_window(*this);

	m_panel->m_config.m_code = m_scintilla.GetCode();
	m_panel->Update();

	if (nID == IDC_BTN_APPLY)
	{
		m_scintilla.SetSavePoint();
	}
	else if (nID == IDOK)
	{
		EndDialog(nID);
	}
}

void CDialogConfigure::OnCancel(uint32_t, int nID, CWindow)
{
	if (!m_scintilla.GetModify() || popup_message_v3::get()->messageBox(m_hWnd, "Unsaved changes will be lost. Are you sure?", Component::name.data(), MB_YESNO) == IDYES)
	{
		EndDialog(nID);
	}
}

void CDialogConfigure::OnSamples(uint32_t, int, CWindow)
{
	CRect rect;
	HMENU samples = CreatePopupMenu();
	HMENU basic = CreatePopupMenu();
	std::wstring sample_file;

	for (auto&& [index, path] : std::views::enumerate(m_basic))
	{
		AppendMenuW(basic, MF_STRING, ID_MENU_BASIC_BEGIN + index, Path::filename(path).data());
	}

	for (auto&& [index, path] : std::views::enumerate(m_samples))
	{
		AppendMenuW(samples, MF_STRING, ID_MENU_SAMPLES_BEGIN + index, Path::filename(path).data());
	}

	InsertMenuW(samples, ID_MENU_SAMPLES_BEGIN, MF_STRING | MF_POPUP, reinterpret_cast<uintptr_t>(basic), L"basic");

	GetDlgItem(IDC_BTN_SAMPLES).GetWindowRect(&rect);
	const int id = TrackPopupMenuEx(samples, TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.top, m_hWnd, nullptr);
	DestroyMenu(samples);

	if (id >= ID_MENU_SAMPLES_BEGIN && id <= ID_MENU_SAMPLES_END)
	{
		sample_file = m_samples[id - ID_MENU_SAMPLES_BEGIN];

	}
	else if (id >= ID_MENU_BASIC_BEGIN && id <= ID_MENU_BASIC_END)
	{
		sample_file = m_basic[id - ID_MENU_BASIC_BEGIN];
	}
	else
	{
		return;
	}

	const std::string str = TextFile(sample_file).read();
	m_scintilla.SetCode(str);
}

void CDialogConfigure::OnStyle(uint32_t, int, CWindow)
{
	CRect rect;
	HMENU menu = CreatePopupMenu();
	uint32_t check{}, edit_flag{};

	if (m_scintilla.m_mode == ScintillaConfig::Mode::JavaScriptCustom)
	{
		check = ID_MENU_STYLE_CUSTOM;
		edit_flag = MF_STRING;
	}
	else
	{
		check = ID_MENU_STYLE_AUTO;
		edit_flag = MF_GRAYED;
	}

	AppendMenuW(menu, MF_STRING, ID_MENU_STYLE_AUTO, L"Auto");
	AppendMenuW(menu, MF_STRING, ID_MENU_STYLE_CUSTOM, L"Custom");
	AppendMenuW(menu, MF_SEPARATOR, 0, 0);
	AppendMenuW(menu, edit_flag, ID_MENU_STYLE_EDIT, L"Edit...");
	CheckMenuRadioItem(menu, ID_MENU_STYLE_AUTO, ID_MENU_STYLE_CUSTOM, check, MF_BYCOMMAND);

	GetDlgItem(IDC_BTN_STYLE).GetWindowRect(&rect);
	const int id = TrackPopupMenuEx(menu, TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.top, m_hWnd, nullptr);
	DestroyMenu(menu);

	switch (id)
	{
	case ID_MENU_STYLE_AUTO:
		m_scintilla.SetMode(ScintillaConfig::Mode::JavaScriptAuto);
		break;
	case ID_MENU_STYLE_CUSTOM:
		m_scintilla.SetMode(ScintillaConfig::Mode::JavaScriptCustom);
		break;
	case ID_MENU_STYLE_EDIT:
		m_scintilla.OpenStyleDialog();
		break;
	}
}

void CDialogConfigure::OnTools(uint32_t, int, CWindow)
{
	CRect rect;
	HMENU menu = CreatePopupMenu();

	AppendMenuW(menu, MF_STRING, ID_MENU_RESET, L"Reset");
	AppendMenuW(menu, MF_SEPARATOR, 0, 0);
	AppendMenuW(menu, MF_STRING, ID_MENU_IMPORT, L"Import");
	AppendMenuW(menu, MF_STRING, ID_MENU_EXPORT, L"Export");
	AppendMenuW(menu, MF_SEPARATOR, 0, 0);
	AppendMenuW(menu, MF_STRING, ID_MENU_DOCS, L"Docs");
	AppendMenuW(menu, MF_STRING, ID_MENU_RELEASES, L"Releases");
	AppendMenuW(menu, MF_SEPARATOR, 0, 0);
	AppendMenuW(menu, MF_STRING, ID_MENU_LICENSES, L"Licenses");
	AppendMenuW(menu, MF_STRING, ID_MENU_ABOUT, L"About");

	GetDlgItem(IDC_BTN_TOOLS).GetWindowRect(&rect);
	const int id = TrackPopupMenuEx(menu, TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.top, m_hWnd, nullptr);
	DestroyMenu(menu);

	switch (id)
	{
	case ID_MENU_RESET:
		m_scintilla.SetCode(m_panel->m_config.get_default_script());
		break;
	case ID_MENU_IMPORT:
		m_scintilla.Import();
		break;
	case ID_MENU_EXPORT:
		m_scintilla.Export();
		break;
	case ID_MENU_DOCS:
		Utils::execute(Component::Docs);
		break;
	case ID_MENU_RELEASES:
		Utils::execute(Component::Releases);
		break;
	case ID_MENU_LICENSES:
		Utils::execute(m_licenses_folder);
		break;
	case ID_MENU_ABOUT:
		Component::popup(Component::about());
		break;
	}
}
