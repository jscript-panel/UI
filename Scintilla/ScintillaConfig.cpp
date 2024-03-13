#include "stdafx.hpp"
#include "ScintillaConfig.hpp"

namespace
{
	static constexpr std::array config_styles =
	{
		"style.default",
		"style.comment",
		"style.keyword",
		"style.identifier",
		"style.string",
		"style.number",
		"style.operator",
		"style.bracelight",
		"style.bracebad",
		"style.caret.fore",
		"style.selection.back",
	};

	cfg_int cfg_int_scintilla_mode(guids::cfg_int_scintilla_mode, 0);
	cfg_int cfg_int_scintilla_zoom(guids::cfg_int_scintilla_zoom, 0);
	cfg_string cfg_string_scintilla(guids::cfg_string_scintilla, "");
}

ScintillaConfig g_scintilla_config;

#pragma region static
ScintillaConfig::Data ScintillaConfig::cfg_string_to_data(wil::zstring_view str)
{
	Data data;
	StringMap map;

	for (auto&& line : split_string(str, CRLF))
	{
		const Strings tmp = split_string(line, "=");
		if (tmp.size() == 2)
		{
			map.emplace(tmp[0], tmp[1]);
		}
	}

	for (auto&& name : config_styles)
	{
		std::string value;
		const auto it = map.find(name);
		if (it != map.end())
		{
			value = it->second;
		}
		data.emplace_back(StringPair(name, value));
	}

	return data;
}

ScintillaConfig::Data ScintillaConfig::get_default_data()
{
	if (ui_config_manager::g_is_dark_mode())
	{
		return preset_to_data(IDR_CFG_DARK_GRAY);
	}
	return preset_to_data(IDR_CFG_DEFAULT);
}

ScintillaConfig::Data ScintillaConfig::preset_to_data(int id)
{
	const std::string str = Component::get_resource_text(id);
	return cfg_string_to_data(str);
}

int ScintillaConfig::get_mode()
{
	int mode = cfg_int_scintilla_mode;
	if (mode == 0) // first run
	{
		mode = Fb::is_v2() ? 1 : 2;
		set_mode(mode);
	}
	return mode;
}

int ScintillaConfig::get_zoom()
{
	return cfg_int_scintilla_zoom;
}

std::string ScintillaConfig::data_to_string(const Data& data)
{
	fmt::memory_buffer buffer;
	for (const auto& [name, value] : data)
	{
		fmt::format_to(std::back_inserter(buffer), "{}={}{}", name, value, CRLF);
	}
	return fmt::to_string(buffer);
}

void ScintillaConfig::set_mode(int mode)
{
	cfg_int_scintilla_mode = mode;
}

void ScintillaConfig::set_zoom(int zoom)
{
	cfg_int_scintilla_zoom = zoom;
}
#pragma endregion

void ScintillaConfig::export_to_file(wil::zstring_view path)
{
	const std::string str = data_to_string(m_data);
	FileHelper(path).write(str);
}

void ScintillaConfig::import_from_file(wil::zstring_view path)
{
	cfg_string_scintilla = FileHelper(path).read();
	m_data = cfg_string_to_data(cfg_string_scintilla);
}

void ScintillaConfig::init_data()
{
	if (cfg_string_scintilla.is_empty())
	{
		m_data = get_default_data();
		set_data();
	}
	else
	{
		m_data = cfg_string_to_data(cfg_string_scintilla);
	}
}

void ScintillaConfig::load_preset(int id)
{
	m_data = preset_to_data(id);
	set_data();
}

void ScintillaConfig::set_data()
{
	cfg_string_scintilla = data_to_string(m_data);
}

void ScintillaConfig::set_data_item(size_t idx, wil::zstring_view str)
{
	m_data[idx].second = str;
	set_data();
}

FB2K_ON_INIT_STAGE([] { g_scintilla_config.init_data(); }, init_stages::after_config_read)
