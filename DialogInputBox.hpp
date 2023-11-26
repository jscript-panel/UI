#pragma once

class CDialogInputBox : public CDialogImpl<CDialogInputBox>
{
public:
	CDialogInputBox(wil::zwstring_view prompt, wil::zwstring_view title, wil::zwstring_view value);

	BEGIN_MSG_MAP_EX(CDialogInputBox)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_INPUTBOX };

	std::wstring m_value;

private:
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnCloseCmd(uint32_t, int nID, CWindow);

	CEdit m_edit_value;
	fb2k::CCoreDarkModeHooks m_hooks;
	std::wstring m_title, m_prompt;
};
