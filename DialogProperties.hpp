#pragma once
#include "PropertyList.hpp"

class CDialogProperties : public CDialogImpl<CDialogProperties>
{
public:
	CDialogProperties(PanelBase* panel);

	BEGIN_MSG_MAP_EX(CDialogProperties)
		CHAIN_MSG_MAP_MEMBER(m_resizer)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		COMMAND_ID_HANDLER_EX(IDC_BTN_IMPORT, OnImport)
		COMMAND_ID_HANDLER_EX(IDC_BTN_EXPORT, OnExport)
		COMMAND_ID_HANDLER_EX(IDC_BTN_CLEAR, OnClear)
		COMMAND_ID_HANDLER_EX(IDOK, OnApplyOrOK)
		COMMAND_ID_HANDLER_EX(IDC_BTN_APPLY, OnApplyOrOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_PROPERTIES };

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnApplyOrOK(uint32_t, int nID, CWindow);
	void OnCancel(uint32_t, int nID, CWindow);
	void OnClear(uint32_t, int, CWindow);
	void OnContextMenu(CWindow, CPoint pt);
	void OnExport(uint32_t, int, CWindow);
	void OnImport(uint32_t, int, CWindow);

	CButton m_btn_clear, m_btn_export;
	CDialogResizeHelper m_resizer;
	PanelBase* m_panel;
	PropertyList m_list;
	fb2k::CCoreDarkModeHooks m_hooks;
};
