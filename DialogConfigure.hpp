#pragma once
#include "Scintilla.hpp"

class PanelBase;

class CDialogConfigure : public CDialogImpl<CDialogConfigure>
{
public:
	CDialogConfigure(PanelBase* panel);

	BEGIN_MSG_MAP_EX(CDialogConfigure)
		CHAIN_MSG_MAP_MEMBER(m_resizer)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_NOTIFY(OnNotify)
		COMMAND_ID_HANDLER_EX(IDC_BTN_TOOLS, OnTools)
		COMMAND_ID_HANDLER_EX(IDC_BTN_STYLE, OnStyle)
		COMMAND_ID_HANDLER_EX(IDC_BTN_SAMPLES, OnSamples)
		COMMAND_ID_HANDLER_EX(IDOK, OnApplyOrOK)
		COMMAND_ID_HANDLER_EX(IDC_BTN_APPLY, OnApplyOrOK)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_CONFIGURE };

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	LRESULT OnNotify(int, LPNMHDR pnmh);
	void InitControls();
	void InitFolders();
	void InitScintilla();
	void InitTitle();
	void OnApplyOrOK(uint32_t, int nID, CWindow);
	void OnCancel(uint32_t, int nID, CWindow);
	void OnSamples(uint32_t, int, CWindow);
	void OnStyle(uint32_t, int, CWindow);
	void OnTools(uint32_t, int, CWindow);

	CComboBox m_combo_edge;
	CDialogResizeHelper m_resizer;
	CScintilla m_scintilla;
	PanelBase* m_panel;
	WStrings m_basic, m_samples;
	fb2k::CCoreDarkModeHooks m_hooks;
	std::string m_title;
	std::wstring m_licenses_folder;
};
