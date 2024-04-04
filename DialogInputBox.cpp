#include "stdafx.hpp"
#include "DialogInputBox.hpp"

CDialogInputBox::CDialogInputBox(wil::zwstring_view prompt, wil::zwstring_view title, wil::zwstring_view value) : m_prompt(prompt), m_title(title), m_value(value) {}

BOOL CDialogInputBox::OnInitDialog(CWindow, LPARAM)
{
	SetWindowTextW(m_title.data());
	GetDlgItem(IDC_LABEL_PROMPT).SetWindowTextW(m_prompt.data());

	m_edit_value = GetDlgItem(IDC_EDIT_VALUE);
	m_edit_value.SetWindowTextW(m_value.data());
	PP::editBoxFix(m_edit_value);

	m_hooks.AddDialogWithControls(*this);
	CenterWindow();
	return TRUE;
}

void CDialogInputBox::OnCloseCmd(uint32_t, int nID, CWindow)
{
	m_value = js::to_wide(pfc::getWindowText(m_edit_value));
	EndDialog(nID);
}
