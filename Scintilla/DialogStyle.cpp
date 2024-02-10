#include "stdafx.hpp"
#include "DialogStyle.hpp"

CDialogStyle::CDialogStyle(CScintilla* parent) : m_parent(parent), m_list(this) {}

#pragma region IListControlOwnerDataSource
bool CDialogStyle::listIsColumnEditable(ctx_t, size_t column)
{
	return column == 1;
}

size_t CDialogStyle::listGetItemCount(ctx_t)
{
	return g_scintilla_config.m_data.size();
}

string8 CDialogStyle::listGetSubItemText(ctx_t, size_t row, size_t column)
{
	switch (column)
	{
	case 0:
		return g_scintilla_config.m_data[row].first;
	case 1:
		return g_scintilla_config.m_data[row].second;
	}
	return "";
}

void CDialogStyle::listSetEditField(ctx_t, size_t row, size_t column, const char* value)
{
	if (column == 1)
	{
		g_scintilla_config.set_data_item(row, value);
		m_parent->SetStyles();
	}
}

void CDialogStyle::listSubItemClicked(ctx_t, size_t row, size_t column)
{
	if (column == 1)
	{
		m_list.TableEdit_Start(row, column);
	}
}
#pragma endregion

BOOL CDialogStyle::OnInitDialog(CWindow, LPARAM)
{
	m_list.CreateInDialog(*this, IDC_LIST_STYLE);
	m_list.SetWindowLongPtrW(GWL_EXSTYLE, 0L);
	m_list.InitializeHeaderCtrl(HDS_NOSIZING);
	m_list.SetSelectionModeNone();

	const auto dpi = m_list.GetDPI().cx;
	m_list.AddColumn("Name", MulDiv(150, dpi, 96));
	m_list.AddColumnAutoWidth("Value");

	m_hooks.AddDialogWithControls(*this);
	return TRUE;
}

void CDialogStyle::OnCancel(uint32_t, int nID, CWindow)
{
	EndDialog(nID);
}

void CDialogStyle::OnExportBnClicked(uint32_t, int, CWindow)
{
	string8 path;
	if (uGetOpenFileName(m_hWnd, "Configuration files|*.cfg", 0, "cfg", "Save as", nullptr, path, TRUE))
	{
		g_scintilla_config.export_to_file(path);
	}
}

void CDialogStyle::OnImportBnClicked(uint32_t, int, CWindow)
{
	string8 path;
	if (uGetOpenFileName(m_hWnd, "Configuration files|*.cfg|All files|*.*", 0, "cfg", "Import from", nullptr, path, FALSE))
	{
		g_scintilla_config.import_from_file(path);
		m_parent->SetStyles();
		m_list.ReloadData();
	}
}

void CDialogStyle::OnPresetsBnClicked(uint32_t, int, CWindow)
{
	HMENU menu = CreatePopupMenu();
	AppendMenuW(menu, MF_STRING, IDR_CFG_DEFAULT, L"Default");
	AppendMenuW(menu, MF_STRING, IDR_CFG_BRIGHT, L"Bright");
	AppendMenuW(menu, MF_STRING, IDR_CFG_DARK_GRAY, L"Dark Gray");
	AppendMenuW(menu, MF_STRING, IDR_CFG_RUBY_BLUE, L"Ruby Blue");

	CRect rect;
	GetDlgItem(IDC_BTN_PRESETS).GetWindowRect(&rect);
	const int id = TrackPopupMenuEx(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.bottom, m_hWnd, nullptr);
	DestroyMenu(menu);

	if (id > 0)
	{
		g_scintilla_config.load_preset(id);
		m_parent->SetStyles();
		m_list.ReloadData();
	}
}
