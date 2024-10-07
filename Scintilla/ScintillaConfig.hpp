#pragma once

class ScintillaConfig
{
public:
	enum class Mode
	{
		PlainText,
		JavaScriptAuto,
		JavaScriptCustom,
	};

	using Data = StringPairs;

	static Data cfg_string_to_data(std::string_view str);
	static Data get_default_data();
	static Data preset_to_data(int id);
	static Mode get_mode();
	static int64_t get_zoom();
	static std::string data_to_string(const Data& data);
	static void set_mode(Mode mode);
	static void set_zoom(int64_t zoom);

	void export_to_file(std::string_view path);
	void import_from_file(std::string_view path);
	void init_data();
	void load_preset(int id);
	void set_data();
	void set_data_item(size_t idx, std::string_view str);

	Data m_data;
};

extern ScintillaConfig g_scintilla_config;
