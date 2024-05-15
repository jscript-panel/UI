#pragma once
#include "ScintillaConfig.hpp"
#include "ScintillaImpl.hpp"

class CDialogFindReplace;

class CScintilla : public CScintillaImpl<CScintilla>, public ui_config_callback_impl
{
public:
	enum class Mode : int64_t
	{
		PlainText,
		JavaScriptAuto,
		JavaScriptCustom,
	};

	BEGIN_MSG_MAP_EX(CScintilla)
		MSG_WM_KEYDOWN(OnKeyDown)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(static_cast<uint32_t>(Notification::UpdateUI), OnUpdateUI)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(static_cast<uint32_t>(Notification::CharAdded), OnCharAdded)
		REFLECTED_NOTIFY_CODE_HANDLER_EX(static_cast<uint32_t>(Notification::Zoom), OnZoom)
		REFLECTED_COMMAND_CODE_HANDLER_EX(static_cast<uint32_t>(FocusChange::Change), OnChange)
	END_MSG_MAP()

	bool Find(bool next);
	std::string GetCode();
	void Export();
	void Import();
	void Init(Mode mode);
	void OpenStyleDialog();
	void Replace();
	void ReplaceAll();
	void SetCode(wil::zstring_view code);
	void SetMode(Mode mode);
	void SetStyles();

	// ui_config_callback_impl
	void ui_colors_changed() final;

	Mode m_mode{};

private:
	struct API
	{
		std::string text;
		size_t len{};
	};

	struct EditorStyle
	{
		bool bold{}, italic{};
		int size{};
		std::optional<Colour> back{}, fore{};
		std::string font;
	};

	struct Range
	{
		Position start{}, end{};
	};

	COLORREF GetSysColour(int id);
	ColourAlpha GetSysColourAlpha(int id);
	Line GetCurrentLineNumber();
	LRESULT OnChange(uint32_t, int, CWindow);
	LRESULT OnCharAdded(LPNMHDR);
	LRESULT OnUpdateUI(LPNMHDR);
	LRESULT OnZoom(LPNMHDR);
	Position GetCaretInLine();
	Range GetSelection();
	ScintillaConfig::Data GetConfigData();
	bool IsWordCharacter(char c);
	std::optional<int> ParseHex(wil::zstring_view hex, bool alpha = false);
	std::string GetCurLineText();
	std::string GetLineText(Line line);
	std::string GetSelectedText();
	std::string GetWordStart(wil::zstring_view text, Position current);
	void AutoMarginWidth();
	void AutomaticIndentation(int ch);
	void ContinueCallTip();
	void InitJS();
	void InitKeys();
	void InitMargins();
	void InitZoom();
	void OnKeyDown(uint32_t ch, uint32_t, uint32_t);
	void OpenFindDialog(wil::zstring_view selected_text = "");
	void OpenGotoDialog();
	void OpenReplaceDialog(wil::zstring_view selected_text = "");
	void ReadAPIs();
	void SetIndentation(Line line, int indent);
	void SetStyle(wil::zstring_view name, wil::zstring_view value);
	void StartAutoComplete();
	void StartCallTip();
	void TrackWidth();

	CDialogFindReplace* m_dlg_find_replace{};
	Position m_word_start_pos{};
	bool m_is_dark;
	int m_brace_count{};
	std::string m_function_definition;
	std::vector<API> m_apis;
};
