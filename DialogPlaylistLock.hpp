#pragma once

class CDialogPlaylistLock : public CDialogImpl<CDialogPlaylistLock>
{
public:
	CDialogPlaylistLock(size_t playlistIndex);

	BEGIN_MSG_MAP_EX(CDialogPlaylistLock)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_PLAYLIST_LOCK };

private:
	struct Item
	{
		CCheckBox cbox;
		uint32_t filter{};
	};

	BOOL OnInitDialog(CWindow, LPARAM);
	void OnCloseCmd(uint32_t, int nID, CWindow);

	fb2k::CCoreDarkModeHooks m_hooks;
	size_t m_playlistIndex{};
	std::vector<Item> m_items;
	uint32_t m_mask{};
};
