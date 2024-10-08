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
ScintillaConfig::Data ScintillaConfig::cfg_string_to_data(std::string_view str)
{
	Data data;
	StringMap map;

	for (auto&& line : js::split_string(str, CRLF))
	{
		const auto tmp = js::split_string(line, "=");

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

		data.emplace_back(name, value);
	}

	return data;
}

ScintillaConfig::Data ScintillaConfig::get_default_data()
{
	if (ui_config_manager::g_is_dark_mode())
		return preset_to_data(IDR_CFG_DARK_GRAY);

	return preset_to_data(IDR_CFG_DEFAULT);
}

ScintillaConfig::Data ScintillaConfig::preset_to_data(int id)
{
	const std::string str = js::get_resource_text(id);
	return cfg_string_to_data(str);
}

ScintillaConfig::Mode ScintillaConfig::get_mode()
{
	auto mode = static_cast<Mode>(cfg_int_scintilla_mode.get());

	if (mode == Mode::PlainText)
	{
		mode = Mode::JavaScriptAuto;
		set_mode(mode);
	}

	return mode;
}

int64_t ScintillaConfig::get_zoom()
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

void ScintillaConfig::set_mode(Mode mode)
{
	cfg_int_scintilla_mode = std::to_underlying(mode);
}

void ScintillaConfig::set_zoom(int64_t zoom)
{
	cfg_int_scintilla_zoom = zoom;
}
#pragma endregion

void ScintillaConfig::export_to_file(std::string_view path)
{
	const std::string str = data_to_string(m_data);
	TextFile(path).write(str);
}

void ScintillaConfig::import_from_file(std::string_view path)
{
	cfg_string_scintilla = TextFile(path).read().c_str();
	m_data = cfg_string_to_data(cfg_string_scintilla.get());
}

void ScintillaConfig::init_data()
{
	if (cfg_string_scintilla.get().is_empty())
	{
		m_data = get_default_data();
		set_data();
	}
	else
	{
		m_data = cfg_string_to_data(cfg_string_scintilla.get());
	}
}

void ScintillaConfig::load_preset(int id)
{
	m_data = preset_to_data(id);
	set_data();
}

void ScintillaConfig::set_data()
{
	cfg_string_scintilla = data_to_string(m_data).c_str();
}

void ScintillaConfig::set_data_item(size_t idx, std::string_view str)
{
	m_data[idx].second = str;
	set_data();
}

