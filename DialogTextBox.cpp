#include "stdafx.hpp"
#include "DialogTextBox.hpp"

CDialogTextBox::CDialogTextBox(std::wstring_view prompt, std::wstring_view title, std::wstring_view value) : m_prompt(prompt), m_title(title), m_value(value) {}

BOOL CDialogTextBox::OnInitDialog(CWindow, LPARAM)
{
	SetWindowTextW(m_title.data());
	GetDlgItem(IDC_LABEL_PROMPT).SetWindowTextW(m_prompt.data());

	m_scintilla.SubclassWindow(GetDlgItem(IDC_SCINTILLA));
	m_scintilla.Init(ScintillaConfig::Mode::PlainText);
	m_scintilla.SetCode(js::from_wide(m_value));

	m_hooks.AddDialogWithControls(*this);
	CenterWindow();
	return TRUE;
}

void CDialogTextBox::OnCloseCmd(uint32_t, int nID, CWindow)
{
	if (nID == IDOK)
	{
		m_value = js::to_wide(m_scintilla.GetCode());
	}

	EndDialog(nID);
}
