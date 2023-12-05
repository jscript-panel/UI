#include "stdafx.hpp"
#include "ColourPickerHook.hpp"

#include <libPPUI/DarkMode.h>

ColourPickerHook::ColourPickerHook(bool is_dark) : m_is_dark(is_dark) {}

HBRUSH ColourPickerHook::OnCtlColor(CDCHandle dc, CWindow)
{
	if (m_is_dark)
	{
		static constexpr COLORREF bkColor = 0x202020;
		static constexpr COLORREF txtColor = 0xC0C0C0;

		dc.SetTextColor(txtColor);
		dc.SetBkColor(bkColor);
		dc.SetDCBrushColor(bkColor);
		return (HBRUSH)GetStockObject(DC_BRUSH);
	}

	SetMsgHandled(FALSE);
	return nullptr;
}

uintptr_t __stdcall ColourPickerHook::HookProc(HWND wnd, uint32_t msg, WPARAM, LPARAM lp)
{
	if (msg == WM_INITDIALOG)
	{
		auto cc = reinterpret_cast<CHOOSECOLOR*>(lp);
		auto scope = reinterpret_cast<modal_dialog_scope*>(cc->lCustData);
		scope->initialize(pfc::findOwningPopup(wnd));

		const bool is_dark = ui_config_manager::g_is_dark_mode();
		PP::subclassThisWindow<ColourPickerHook>(wnd, is_dark);
		UpdateControls(wnd, is_dark);
	}
	return 0;
}

void ColourPickerHook::UpdateControls(CWindow wnd, bool is_dark)
{
	DarkMode::UpdateTitleBar(wnd, is_dark);

	for (CWindow child = wnd.GetWindow(GW_CHILD); child.m_hWnd != nullptr; child = child.GetWindow(GW_HWNDNEXT))
	{
		Utils::set_window_theme(child, is_dark);
	}
}

void ColourPickerHook::ui_colors_changed()
{
	m_is_dark = ui_config_manager::g_is_dark_mode();

	Invalidate();
	UpdateControls(m_hWnd, m_is_dark);
}
