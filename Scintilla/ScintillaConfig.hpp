#pragma once

class ScintillaConfig
{
public:
	using Data = std::vector<StringPair>;

	static Data cfg_string_to_data(wil::zstring_view str);
	static Data get_default_data();
	static Data preset_to_data(int id);
	static int64_t get_mode();
	static int64_t get_zoom();
	static std::string data_to_string(const Data& data);
	static void set_mode(int64_t mode);
	static void set_zoom(int64_t zoom);

	void export_to_file(wil::zstring_view path);
	void import_from_file(wil::zstring_view path);
	void init_data();
	void load_preset(int id);
	void set_data();
	void set_data_item(size_t idx, wil::zstring_view str);

	Data m_data;
};

extern ScintillaConfig g_scintilla_config;
