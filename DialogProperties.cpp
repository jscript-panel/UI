#include "stdafx.hpp"
#include "DialogProperties.hpp"

namespace
{
	static const CDialogResizeHelper::Param resize_data[] =
	{
		{ IDC_LIST_PROPERTIES, 0, 0, 1, 1 },
		{ IDC_BTN_IMPORT, 0, 1, 0, 1 },
		{ IDC_BTN_EXPORT, 0, 1, 0, 1 },
		{ IDC_BTN_CLEAR, 0, 1, 0, 1 },
		{ IDOK, 1, 1, 1, 1 },
		{ IDCANCEL, 1, 1, 1, 1 },
		{ IDC_BTN_APPLY, 1, 1, 1, 1 },
	};

	static const CRect resize_min_max(620, 400, 0, 0);
}

CDialogProperties::CDialogProperties(PanelBase* panel) : m_panel(panel), m_resizer(resize_data, resize_min_max) {}

BOOL CDialogProperties::OnInitDialog(CWindow, LPARAM)
{
	SetIcon(ui_control::get()->get_main_icon());
	pfc::setWindowText(m_hWnd, fmt::format("{} Properties (id:{})", Component::name, m_panel->m_id).c_str());

	m_list.CreateInDialog(*this, IDC_LIST_PROPERTIES);
	m_list.SetWindowLongPtrW(GWL_EXSTYLE, 0L);
	m_list.SetData(m_panel->m_config.m_property_data);

	m_btn_clear = GetDlgItem(IDC_BTN_CLEAR);
	m_btn_export = GetDlgItem(IDC_BTN_EXPORT);

	m_hooks.AddDialogWithControls(*this);
	Component::window_placement.apply_to_window(*this, false);
	return TRUE;
}

void CDialogProperties::OnApplyOrOK(uint32_t, int nID, CWindow)
{
	m_panel->m_config.m_property_data = m_list.GetData();
	m_panel->update_script();
	m_list.SetData(m_panel->m_config.m_property_data);
	Component::window_placement.read_from_window(*this);

	if (nID == IDC_BTN_APPLY) return;
	EndDialog(nID);
}

void CDialogProperties::OnCancel(uint32_t, int nID, CWindow)
{
	EndDialog(nID);
}

void CDialogProperties::OnClear(uint32_t, int, CWindow)
{
	m_list.SelectAll();
	m_list.RequestRemoveSelection();
}

void CDialogProperties::OnContextMenu(CWindow, CPoint pt)
{
	m_list.OnContextMenu(pt);
}

void CDialogProperties::OnExport(uint32_t, int, CWindow)
{
	string8 path;

	if (uGetOpenFileName(m_hWnd, "Property files|*.wsp", 0, "wsp", "Save as", nullptr, path, TRUE))
	{
		try
		{
			file_ptr io;
			filesystem::g_open_write_new(io, path, fb2k::noAbort);
			PanelConfig::get_properties(io.get_ptr(), m_list.GetData(), fb2k::noAbort);
		}
		catch (...) {}
	}
}

void CDialogProperties::OnImport(uint32_t, int, CWindow)
{
	string8 path;

	if (uGetOpenFileName(m_hWnd, "Property files|*.wsp|All files|*.*", 0, "wsp", "Import from", nullptr, path, FALSE))
	{
		try
		{
			file_ptr io;
			filesystem::g_open_read(io, path, fb2k::noAbort);
			PanelConfig::PropertyData data;
			PanelConfig::set_properties(io.get_ptr(), data, fb2k::noAbort);
			m_list.SetData(data);
		}
		catch (...) {}
	}
}
