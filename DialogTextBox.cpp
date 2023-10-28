#include "stdafx.hpp"
#include "DialogTextBox.hpp"

CDialogTextBox::CDialogTextBox(wil::zwstring_view prompt, wil::zwstring_view caption, wil::zwstring_view value) : m_prompt(prompt), m_caption(caption), m_value(value) {}

BOOL CDialogTextBox::OnInitDialog(CWindow, LPARAM)
{
	SetWindowTextW(m_caption.data());
	GetDlgItem(IDC_LABEL_PROMPT).SetWindowTextW(m_prompt.data());

	m_scintilla.SubclassWindow(GetDlgItem(IDC_SCINTILLA));
	m_scintilla.Init(CScintilla::Mode::PlainText);
	m_scintilla.SetCode(from_wide(m_value));

	m_hooks.AddDialogWithControls(*this);
	CenterWindow();
	return TRUE;
}

void CDialogTextBox::OnCloseCmd(uint32_t, int nID, CWindow)
{
	m_value = to_wide(m_scintilla.GetCode());
	EndDialog(nID);
}
