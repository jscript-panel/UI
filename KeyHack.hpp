#pragma once

class KeyHack : public CWindowImpl<KeyHack, CWindow>
{
public:
	KeyHack(int ret_cmd) : m_ret_cmd(ret_cmd) {}

	BEGIN_MSG_MAP_EX(KeyHack)
		MSG_WM_CHAR(OnChar)
		MSG_WM_KEYDOWN(OnKeyDown)
	END_MSG_MAP()

private:
	void OnChar(uint32_t ch, uint32_t, uint32_t)
	{
		switch (ch)
		{
		case '\n':
		case '\r':
		case '\t':
		case '\x1b':
			return;
		}
		SetMsgHandled(FALSE);
	}

	void OnKeyDown(uint32_t ch, uint32_t, uint32_t)
	{
		switch (ch)
		{
		case VK_RETURN:
			GetParent().PostMessageW(WM_COMMAND, MAKEWPARAM(m_ret_cmd, BN_CLICKED), reinterpret_cast<LPARAM>(m_hWnd));
			return;
		case VK_ESCAPE:
			GetParent().PostMessageW(WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), reinterpret_cast<LPARAM>(m_hWnd));
			return;
		case VK_TAB:
			GetParent().PostMessageW(WM_NEXTDLGCTL, IsKeyPressed(VK_SHIFT) ? 1 : 0);
			return;
		}
		SetMsgHandled(FALSE);
	}

	int m_ret_cmd{};
};
