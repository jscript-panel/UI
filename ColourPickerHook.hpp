#pragma once

class ColourPickerHook : public CWindowImpl<ColourPickerHook>, ui_config_callback_impl
{
public:
	ColourPickerHook(bool is_dark);

	BEGIN_MSG_MAP_EX(ColourPickerHook)
		MSG_WM_CTLCOLORDLG(OnCtlColor)
		MSG_WM_CTLCOLORSTATIC(OnCtlColor)
		MSG_WM_CTLCOLOREDIT(OnCtlColor)
	END_MSG_MAP()

	static uintptr_t __stdcall HookProc(HWND wnd, uint32_t msg, WPARAM, LPARAM lp);

	HBRUSH OnCtlColor(CDCHandle dc, CWindow);
	void UpdateControls();

	void ui_colors_changed() final;

private:
	bool m_is_dark{};
};
