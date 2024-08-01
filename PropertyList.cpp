#include "stdafx.hpp"
#include "PropertyList.hpp"

#pragma region CListControlComplete
bool PropertyList::GetCellCheckState(size_t row, size_t column) const
{
	if (column == 1 && m_items[row].is_bool)
		return m_items[row].bool_value;

	return false;
}

bool PropertyList::GetSubItemText(size_t row, size_t column, pfc::string_base& out) const
{
	switch(column)
	{
	case 0:
		out = m_items[row].key;
		return true;
	case 1:
		if (m_items[row].is_bool)
		{
			return false;
		}
		out = m_items[row].value;
		return true;
	default:
		return false;
	}
}

bool PropertyList::TableEdit_IsColumnEditable(size_t column) const
{
	return column == 1;
}

PropertyList::cellType_t PropertyList::GetCellType(size_t row, size_t column) const
{
	if (column == 1 && m_items[row].is_bool)
	{
		return &PFC_SINGLETON(CListCell_Checkbox);
	}
	return &PFC_SINGLETON(CListCell_Text);
}

size_t PropertyList::GetItemCount() const
{
	return m_items.size();
}

void PropertyList::OnSubItemClicked(size_t row, size_t column, CPoint pt)
{
	if (column == 1 && !m_items[row].is_bool)
	{
		TableEdit_Start(row, column);
		return;
	}
	__super::OnSubItemClicked(row, column, pt);
}

void PropertyList::RequestRemoveSelection()
{
	const pfc::bit_array_bittable mask = GetSelectionMask();
	const size_t old_count = GetItemCount();
	pfc::remove_mask_t(m_items, mask);
	OnItemsRemoved(mask, old_count);

	if (m_items.empty())
	{
		m_btn_clear.EnableWindow(FALSE);
		m_btn_export.EnableWindow(FALSE);
	}
}

void PropertyList::SetCellCheckState(size_t row, size_t column, bool value)
{
	if (column == 1 && m_items[row].is_bool)
	{
		m_items[row].bool_value = value;
		__super::SetCellCheckState(row, column, value);
	}
}

void PropertyList::TableEdit_SetField(size_t row, size_t column, const char* value)
{
	if (column == 1 && !m_items[row].is_bool)
	{
		m_items[row].value = value;
		ReloadItem(row);
	}
}
#pragma endregion

PanelConfig::PropertyData PropertyList::GetData()
{
	PanelConfig::PropertyData data;
	_variant_t dst;

	for (auto&& item : m_items)
	{
		if (item.is_bool)
		{
			data.emplace(item.key, _variant_t(item.bool_value));
		}
		else
		{
			const std::wstring wstr = js::to_wide(item.value);
			auto src = _variant_t(wstr.data());

			if (!item.is_string && SUCCEEDED(VariantChangeType(&dst, &src, 0, VT_R8)))
			{
				data.emplace(item.key, dst);
			}
			else
			{
				data.emplace(item.key, src);
			}
		}
	}

	return data;
}

int PropertyList::OnCreate(LPCREATESTRUCT)
{
	AddColumn("Name", MulDiv(360, m_dpi.cx, 96));
	AddColumnAutoWidth("Value");

	m_btn_clear = GetParent().GetDlgItem(IDC_BTN_CLEAR);
	m_btn_export = GetParent().GetDlgItem(IDC_BTN_EXPORT);
	return 0;
}

void PropertyList::OnContextMenu(CPoint pt)
{
	if (GetSelectedCount() == 0) return;

	HMENU menu = CreatePopupMenu();
	AppendMenuW(menu, MF_STRING, ID_MENU_SELECTALL, L"Select all\tCtrl+A");
	AppendMenuW(menu, MF_STRING, ID_MENU_SELECTNONE, L"Select none");
	AppendMenuW(menu, MF_STRING, ID_MENU_INVERTSEL, L"Invert selection");
	AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(menu, MF_STRING, ID_MENU_REMOVE, L"Remove\tDel");

	pt = GetContextMenuPoint(pt);
	const int id = TrackPopupMenuEx(menu, TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, m_hWnd, nullptr);
	DestroyMenu(menu);

	switch (id)
	{
	case ID_MENU_SELECTALL:
		SelectAll();
		break;
	case ID_MENU_SELECTNONE:
		SelectNone();
		break;
	case ID_MENU_INVERTSEL:
		SetSelection(pfc::bit_array_true(), pfc::bit_array_not(GetSelectionMask()));
		break;
	case ID_MENU_REMOVE:
		RequestRemoveSelection();
		break;
	}
}

void PropertyList::SetData(const PanelConfig::PropertyData& data)
{
	m_items.clear();

	for (const auto& [key, value] : data)
	{
		ListItem item;
		item.key = key;

		if (value.vt == VT_BOOL)
		{
			item.is_bool = true;
			item.bool_value = js::to_bool(value.boolVal);
		}
		else if (value.vt == VT_BSTR)
		{
			item.is_string = true;
			item.value = js::from_wide(value.bstrVal);
		}
		else
		{
			_variant_t var;
			var.ChangeType(VT_BSTR, &value);
			item.value = js::from_wide(var.bstrVal);
		}

		m_items.emplace_back(item);
	}

	ReloadData();
	m_btn_clear.EnableWindow(m_items.size() > 0);
	m_btn_export.EnableWindow(m_items.size() > 0);
}
