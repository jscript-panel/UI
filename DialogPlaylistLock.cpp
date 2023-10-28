#include "stdafx.hpp"
#include "DialogPlaylistLock.hpp"
#include "PlaylistLock.hpp"

CDialogPlaylistLock::CDialogPlaylistLock(size_t playlistIndex) : m_playlistIndex(playlistIndex), m_mask(Plman::api()->playlist_lock_get_filter_mask(playlistIndex)) {}

BOOL CDialogPlaylistLock::OnInitDialog(CWindow, LPARAM)
{
	const std::unordered_map<int, uint32_t> id_filter_map =
	{
		{ IDC_CHECK_FILTER_ADD, playlist_lock::filter_add },
		{ IDC_CHECK_FILTER_REMOVE, playlist_lock::filter_remove },
		{ IDC_CHECK_FILTER_REORDER, playlist_lock::filter_reorder },
		{ IDC_CHECK_FILTER_REPLACE, playlist_lock::filter_replace },
		{ IDC_CHECK_FILTER_RENAME, playlist_lock::filter_rename },
		{ IDC_CHECK_FILTER_REMOVE_PLAYLIST, playlist_lock::filter_remove_playlist },
	};

	for (const auto& [id, filter] : id_filter_map)
	{
		Item item(CCheckBox(GetDlgItem(id)), filter);
		item.cbox.SetCheck(WI_IsAnyFlagSet(m_mask, filter));
		m_items.emplace_back(item);
	}

	m_hooks.AddDialogWithControls(*this);
	CenterWindow();
	return TRUE;
}

void CDialogPlaylistLock::OnCloseCmd(uint32_t, int nID, CWindow)
{
	if (nID == IDOK)
	{
		uint32_t mask{};

		for (const auto& [cbox, filter] : m_items)
		{
			if (cbox.IsChecked()) mask |= filter;
		}

		if (mask != m_mask)
		{
			PlaylistLock::remove(m_playlistIndex);
			PlaylistLock::add(m_playlistIndex, mask);
		}
	}

	EndDialog(nID);
}
