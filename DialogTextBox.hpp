#pragma once
#include "Scintilla.hpp"

class CDialogTextBox : public CDialogImpl<CDialogTextBox>
{
public:
	CDialogTextBox(std::wstring_view prompt, std::wstring_view title, std::wstring_view value);

	BEGIN_MSG_MAP_EX(CDialogTextBox)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_TEXTBOX };

	std::wstring m_value;

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnCloseCmd(uint32_t, int nID, CWindow);

	CScintilla m_scintilla;
	fb2k::CCoreDarkModeHooks m_hooks;
	std::wstring m_title, m_prompt;
};
