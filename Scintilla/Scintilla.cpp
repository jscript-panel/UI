#include "stdafx.hpp"
#include "Scintilla.hpp"

#include "DialogFindReplace.hpp"
#include "DialogGoto.hpp"
#include "DialogStyle.hpp"

#include <ILexer.h>
#include <Lexilla.h>
#include <SciLexer.h>
#include <Scintilla.h>

namespace
{
	using StyleIDs = std::vector<int>;

	static const std::map<wil::zstring_view, StyleIDs> style_map =
	{
		{ "style.default", { STYLE_DEFAULT } },
		{ "style.comment", { SCE_C_COMMENT, SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR } },
		{ "style.keyword", { SCE_C_WORD } },
		{ "style.identifier", { SCE_C_IDENTIFIER } },
		{ "style.string", { SCE_C_STRING, SCE_C_CHARACTER } },
		{ "style.number", { SCE_C_NUMBER } },
		{ "style.operator", { SCE_C_OPERATOR } },
		{ "style.bracelight", { STYLE_BRACELIGHT } },
		{ "style.bracebad" , { STYLE_BRACEBAD } },
	};

	FB2K_RUN_ON_INIT_QUIT([] { Scintilla_RegisterClasses(core_api::get_my_instance()); }, [] { Scintilla_ReleaseResources(); });
}

COLORREF CScintilla::GetSysColour(int id)
{
	if (m_is_dark)
	{
		if (id == COLOR_WINDOW || id == COLOR_BTNFACE) return 0x202020;
		else if (id == COLOR_WINDOWTEXT || id == COLOR_BTNTEXT) return 0xC0C0C0;
		else if (id == COLOR_HIGHLIGHT) return 0x777777;
		else if (id == COLOR_HIGHLIGHTTEXT) return 0x101010;
	}
	return GetSysColor(id);
}

ColourAlpha CScintilla::GetSysColourAlpha(int id)
{
	return 0xff000000 | GetSysColour(id);
}

Line CScintilla::GetCurrentLineNumber()
{
	return LineFromPosition(GetCurrentPos());
}

LRESULT CScintilla::OnChange(uint32_t, int, CWindow)
{
	AutoMarginWidth();
	return 0;
}

LRESULT CScintilla::OnCharAdded(LPNMHDR pnmh)
{
	const Range range = GetSelection();
	auto notification = reinterpret_cast<const NotificationData*>(pnmh);
	const char ch = static_cast<char>(notification->ch);

	if (range.start == range.end && range.start > 0)
	{
		if (CallTipActive())
		{
			switch (ch)
			{
			case ')':
				m_brace_count--;
				if (m_brace_count < 1)
					CallTipCancel();
				else
					StartCallTip();
				break;
			case '(':
				m_brace_count++;
				StartCallTip();
				break;
			default:
				ContinueCallTip();
				break;
			}
		}
		else if (AutoCActive())
		{
			if (ch == '(')
			{
				m_brace_count++;
				StartCallTip();
			}
			else if (ch == ')')
			{
				m_brace_count--;
			}
			else if (!IsWordCharacter(ch))
			{
				AutoCCancel();

				if (ch == '.')
					StartAutoComplete();
			}
		}
		else
		{
			if (ch == '(')
			{
				m_brace_count = 1;
				StartCallTip();
			}
			else
			{
				AutomaticIndentation(ch);

				if (IsWordCharacter(ch) || ch == '.')
					StartAutoComplete();
			}
		}
	}

	return 0;
}

LRESULT CScintilla::OnUpdateUI(LPNMHDR)
{
	constexpr auto IsBraceChar = [](int ch)
		{
			return ch == '[' || ch == ']' || ch == '(' || ch == ')' || ch == '{' || ch == '}';
		};

	Position brace_at_caret = -1;
	Position brace_opposite = -1;
	const Position pos = GetCurrentPos();
	const Position len = GetLength();

	if (len > 0)
	{
		if (pos > 0)
		{
			const Position pos_before = PositionBefore(pos);
			if (pos_before == pos - 1 && IsBraceChar(GetCharAt(pos_before)))
			{
				brace_at_caret = pos - 1;
			}
		}

		if (brace_at_caret < 0 && pos < len && IsBraceChar(GetCharAt(pos)))
		{
			brace_at_caret = pos;
		}

		if (brace_at_caret >= 0)
		{
			brace_opposite = BraceMatch(brace_at_caret, 0);
		}

		if (brace_at_caret != -1 && brace_opposite == -1)
		{
			BraceBadLight(brace_at_caret);
			SetHighlightGuide(0);
		}
		else
		{
			BraceHighlight(brace_at_caret, brace_opposite);
			SetHighlightGuide(std::min(GetColumn(brace_at_caret), GetColumn(brace_opposite)));
		}
	}
	return 0;
}

LRESULT CScintilla::OnZoom(LPNMHDR)
{
	g_scintilla_config.set_zoom(GetZoom());
	AutoMarginWidth();
	return 0;
}

CScintilla::Range CScintilla::GetSelection()
{
	Range range;
	range.start = GetSelectionStart();
	range.end = GetSelectionEnd();
	return range;
}

Position CScintilla::GetCaretInLine()
{
	return GetCurrentPos() - PositionFromLine(GetCurrentLineNumber());
}

ScintillaConfig::Data CScintilla::GetConfigData()
{
	if (m_mode == ScintillaConfig::Mode::JavaScriptCustom)
	{
		return g_scintilla_config.m_data;
	}
	return g_scintilla_config.get_default_data();
}

bool CScintilla::Find(bool next)
{
	Position pos{};
	const FindOption flags = m_dlg_find_replace->m_flags;
	const std::string text = m_dlg_find_replace->m_find_text;

	if (next)
	{
		CharRight();
		SearchAnchor();
		pos = SearchNext(flags, text.c_str());
	}
	else
	{
		if (FlagSet(flags, FindOption::RegExp)) return false;

		SearchAnchor();
		pos = SearchPrev(flags, text.c_str());
	}

	if (pos == -1)
	{
		const std::string msg = fmt::format("Cannot find \"{}\"", text);
		popup_message_v3::get()->messageBox(m_dlg_find_replace->m_hWnd, msg.c_str(), Component::name.data(), MB_OK);
		return false;
	}

	ScrollCaret();
	return true;
}

bool CScintilla::IsWordCharacter(char c)
{
	static constexpr wil::zstring_view word_characters = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	return word_characters.contains(c);
}

std::optional<int> CScintilla::ParseHex(wil::zstring_view hex, bool alpha)
{
	const auto hex_digit_to_int = [](int ch)
		{
			if (ch >= '0' && ch <= '9')
			{
				return ch - '0';
			}
			else if (ch >= 'A' && ch <= 'F')
			{
				return ch - 'A' + 10;
			}
			else if (ch >= 'a' && ch <= 'f')
			{
				return ch - 'a' + 10;
			}
			return 0;
		};

	if (hex.length() == 7 && hex.at(0) == '#')
	{
		const int r = hex_digit_to_int(hex.at(1)) << 4 | hex_digit_to_int(hex.at(2));
		const int g = hex_digit_to_int(hex.at(3)) << 4 | hex_digit_to_int(hex.at(4));
		const int b = hex_digit_to_int(hex.at(5)) << 4 | hex_digit_to_int(hex.at(6));

		auto c = RGB(r, g, b);
		if (alpha) c |= 0xff000000;
		return js::to_int(c);
	}
	return std::nullopt;
}

std::string CScintilla::GetCode()
{
	const Position len = GetText(0, nullptr);
	std::string text(len, '\0');
	GetText(len, text.data());
	return text;
}

std::string CScintilla::GetCurLineText()
{
	const Position len = GetCurLine(0, nullptr);
	std::string text(len, '\0');
	GetCurLine(len, text.data());
	return text;
}

std::string CScintilla::GetLineText(Line line)
{
	const Position len = GetLine(line, nullptr);
	std::string text(len, '\0');
	GetLine(line, text.data());
	return text;
}

std::string CScintilla::GetSelectedText()
{
	const Position len = GetSelText(0);
	std::string text(len, '\0');
	GetSelText(text.data());
	return text;
}

std::string CScintilla::GetWordStart(wil::zstring_view text, Position current)
{
	m_word_start_pos = current;
	for (const char c : text | std::views::take(current) | std::views::reverse)
	{
		if (IsWordCharacter(c) || c == '.')
			m_word_start_pos--;
		else
			break;
	}
	return std::string(text.substr(m_word_start_pos, current - m_word_start_pos));
}

void CScintilla::AutoMarginWidth()
{
	if (m_mode == ScintillaConfig::Mode::PlainText) return;

	const Line line_count = GetLineCount() * 10;
	const int margin_width = TextWidth(STYLE_LINENUMBER, pfc::format_int(line_count));
	SetMarginWidthN(0, margin_width);
}

void CScintilla::AutomaticIndentation(int ch)
{
	const Line current_line = GetCurrentLineNumber();
	const Line previous_line = current_line - 1;
	const Position line_start = PositionFromLine(current_line);
	const Position selection_start = GetSelectionStart();
	const int indent_size = GetIndent();
	int indent_block = GetLineIndentation(previous_line);

	if (current_line > 0)
	{
		const std::string text = GetLineText(previous_line);

		for (const char c : text | std::views::reverse)
		{
			if (isspace(c)) continue;
			if (c == '{') indent_block += indent_size;
			break;
		}
	}

	if (ch == '}')
	{
		auto view = std::views::iota(line_start, selection_start - 1);
		const bool all_whitespace = std::ranges::all_of(view, [this](const Position pos) { return isspace(GetCharAt(pos)) != 0; });
		if (all_whitespace)
		{
			SetIndentation(current_line, indent_block - indent_size);
		}
	}
	else if ((ch == '\r' || ch == '\n') && selection_start == line_start)
	{
		SetIndentation(current_line, indent_block);
	}
}

void CScintilla::ContinueCallTip()
{
	const std::string text = GetCurLineText();
	const Position current = GetCaretInLine();
	const size_t len = m_function_definition.length();
	int braces{}, commas{};

	for (Position i = m_word_start_pos; i < current; ++i)
	{
		if (text.at(i) == '(')
			braces++;
		else if (text.at(i) == ')' && braces > 0)
			braces--;
		else if (braces == 1 && text.at(i) == ',')
			commas++;
	}

	size_t start_highlight{};
	while (start_highlight < len && m_function_definition.at(start_highlight) != '(')
	{
		start_highlight++;
	}
	if (start_highlight < len && m_function_definition.at(start_highlight) == '(')
		start_highlight++;

	while (start_highlight < len && commas > 0)
	{
		if (m_function_definition.at(start_highlight) == ',')
			commas--;
		if (m_function_definition.at(start_highlight) == ')')
			commas = 0;
		else
			start_highlight++;
	}

	if (start_highlight < len && m_function_definition.at(start_highlight) == ',')
		start_highlight++;

	size_t end_highlight = start_highlight;
	while (end_highlight < len && m_function_definition.at(end_highlight) != ',' && m_function_definition.at(end_highlight) != ')')
	{
		end_highlight++;
	}

	CallTipSetHlt(start_highlight, end_highlight);
}

void CScintilla::Export()
{
	string8 path;
	if (uGetOpenFileName(m_hWnd, "Text files|*.txt|All files|*.*", 0, "txt", "Save as", nullptr, path, TRUE))
	{
		const std::string str = GetCode();
		FileHelper(path).write(str);
	}
}

void CScintilla::Import()
{
	string8 path;
	if (uGetOpenFileName(m_hWnd, "Text files|*.txt|JScript files|*.js|All files|*.*", 0, "txt", "Import from", nullptr, path, FALSE))
	{
		const std::string str = FileHelper(path).read();
		SetCode(str);
	}
}

void CScintilla::Init(ScintillaConfig::Mode mode)
{
	m_mode = mode;
	m_is_dark = ui_config_manager::g_is_dark_mode();

	SetFnPtr();
	SetBackSpaceUnIndents(true);
	SetCodePage(CpUtf8);
	SetEndAtLastLine(true);
	SetEOLMode(EndOfLine::CrLf);
	SetModEventMask(ModificationFlags::InsertText | ModificationFlags::DeleteText | ModificationFlags::Undo | ModificationFlags::Redo);
	SetIndent(4);
	SetScrollWidth(1);
	SetScrollWidthTracking(true);
	SetTabIndents(true);
	SetTabWidth(4);
	SetTechnology(Technology::DirectWrite);
	SetUseTabs(true);
	UsePopUp(PopUp::Text);

	InitKeys();
	InitMargins();
	InitZoom();
	InitJS();

	SetStyles();
}

void CScintilla::InitJS()
{
	if (m_mode == ScintillaConfig::Mode::PlainText) return;

	static constexpr auto js_keywords =
		"break case catch continue default delete do else false for function if in "
		"instanceof new null return switch this throw true try typeof var while "
		"Array Date Error Function JSON Math Number Object RegExp String "
		"console fb plman utils window";

	auto lexer = CreateLexer("cpp");
	SetILexer(lexer);
	SetKeyWords(0, js_keywords);
	ReadAPIs();
	AutoCSetIgnoreCase(true);
}

void CScintilla::InitKeys()
{
	static constexpr std::array<const int, 21> ctrlcodes = { 'Q', 'W', 'E', 'R', 'I', 'O', 'P', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'B', 'N', 'M', 186, 187, 226 };

	for (const int ctrlcode : ctrlcodes)
	{
		ClearCmdKey(MAKELONG(ctrlcode, KeyMod::Ctrl));
	}

	for (int i = 48; i < 122; ++i)
	{
		ClearCmdKey(MAKELONG(i, KeyMod::Ctrl | KeyMod::Shift));
	}

	AssignCmdKey(MAKELONG(Keys::Next, KeyMod::Ctrl), std::to_underlying(Message::ParaDown));
	AssignCmdKey(MAKELONG(Keys::Prior, KeyMod::Ctrl), std::to_underlying(Message::ParaUp));
	AssignCmdKey(MAKELONG(Keys::Next, (KeyMod::Ctrl | KeyMod::Shift)), std::to_underlying(Message::ParaDownExtend));
	AssignCmdKey(MAKELONG(Keys::Prior, (KeyMod::Ctrl | KeyMod::Shift)), std::to_underlying(Message::ParaUpExtend));
	AssignCmdKey(MAKELONG(Keys::Home, KeyMod::Norm), std::to_underlying(Message::VCHomeWrap));
	AssignCmdKey(MAKELONG(Keys::End, KeyMod::Norm), std::to_underlying(Message::LineEndWrap));
	AssignCmdKey(MAKELONG(Keys::Home, KeyMod::Shift), std::to_underlying(Message::VCHomeWrapExtend));
	AssignCmdKey(MAKELONG(Keys::End, KeyMod::Shift), std::to_underlying(Message::LineEndWrapExtend));
}

void CScintilla::InitMargins()
{
	SetMarginTypeN(0, MarginType::Number);
	SetMarginWidthN(0, 0);
	SetMarginWidthN(1, 0);
	SetMarginWidthN(2, 0);
	SetMarginWidthN(3, 0);
	SetMarginWidthN(4, 0);
}

void CScintilla::InitZoom()
{
	const auto zoom = js::to_int(g_scintilla_config.get_zoom());
	if (zoom != 0)
	{
		SetZoom(zoom);
	}
}

void CScintilla::OnKeyDown(uint32_t ch, uint32_t, uint32_t)
{
	const KeyMod modifiers = (IsKeyPressed(VK_SHIFT) ? KeyMod::Shift : KeyMod::Norm) | (IsKeyPressed(VK_CONTROL) ? KeyMod::Ctrl : KeyMod::Norm) | (IsKeyPressed(VK_MENU) ? KeyMod::Alt : KeyMod::Norm);
	const std::string selected_text = GetSelectedText();

	if (WI_IsSingleFlagSetInMask(KeyMod::Ctrl, modifiers))
	{
		switch (ch)
		{
		case '0':
			SetZoom(0);
			break;
		case 'F':
			OpenFindDialog(selected_text);
			break;
		case 'H':
			OpenReplaceDialog(selected_text);
			break;
		case 'G':
			fb2k::inMainThread([this]
				{
					OpenGotoDialog();
				});
			break;
		case 'S':
			GetParent().PostMessageW(WM_COMMAND, MAKEWPARAM(IDC_BTN_APPLY, BN_CLICKED), reinterpret_cast<LPARAM>(m_hWnd));
			break;
		}
	}
	else if (ch == VK_F3 && WI_IsClearOrSingleFlagSetInMask(KeyMod::Shift, modifiers))
	{
		if (m_dlg_find_replace && m_dlg_find_replace->m_find_text.length())
		{
			Find(modifiers == KeyMod::Norm);
		}
	}

	SetMsgHandled(FALSE);
}

void CScintilla::OpenFindDialog(wil::zstring_view selected_text)
{
	if (!m_dlg_find_replace) m_dlg_find_replace = fb2k::newDialogEx<CDialogFindReplace>(m_hWnd, this);
	m_dlg_find_replace->Update(CDialogFindReplace::Mode::Find, selected_text);
}

void CScintilla::OpenGotoDialog()
{
	if (m_mode == ScintillaConfig::Mode::PlainText) return;

	CDialogGoto dlg(GetCurrentLineNumber() + 1);
	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		const Line line = dlg.m_line_number - 1;
		GotoLine(line);
	}
}

void CScintilla::OpenReplaceDialog(wil::zstring_view selected_text)
{
	if (!m_dlg_find_replace) m_dlg_find_replace = fb2k::newDialogEx<CDialogFindReplace>(m_hWnd, this);
	m_dlg_find_replace->Update(CDialogFindReplace::Mode::Replace, selected_text);
}

void CScintilla::OpenStyleDialog()
{
	CDialogStyle dlg(this);
	dlg.DoModal(m_hWnd);
}

void CScintilla::ReadAPIs()
{
	std::string api_text = js::get_resource_text(IDR_JS_API);
	api_text += js::get_resource_text(IDR_FB2K_API);

	for (auto&& line : js::split_string(api_text, CRLF))
	{
		if (line.empty()) continue;

		API item;
		item.text = line;
		item.len = std::min({ line.find('('), line.find(' '), line.length() });

		m_apis.emplace_back(item);
	}

	std::ranges::sort(m_apis, [](const API& a, const API& b)
		{
			return stricmp_utf8(a.text.c_str(), b.text.c_str()) < 0;
		});
}

void CScintilla::Replace()
{
	const Range range = GetSelection();
	SetTargetStart(range.start);
	SetTargetEnd(range.end);
	const std::string replace = m_dlg_find_replace->m_replace_text;
	ReplaceTarget(replace.length(), replace.c_str());
	SetSel(range.start + replace.length(), range.end);
}

void CScintilla::ReplaceAll()
{
	BeginUndoAction();
	SetTargetStart(0);
	SetTargetEnd(0);
	SetSearchFlags(m_dlg_find_replace->m_flags);
	const std::string find = m_dlg_find_replace->m_find_text;
	const std::string replace = m_dlg_find_replace->m_replace_text;

	while (true)
	{
		SetTargetStart(GetTargetEnd());
		SetTargetEnd(GetLength());

		const Position occurrence = SearchInTarget(find.length(), find.c_str());

		if (occurrence == -1)
		{
			MessageBeep(MB_ICONINFORMATION);
			break;
		}

		ReplaceTarget(replace.length(), replace.c_str());
		SetSel(occurrence + replace.length(), occurrence);
	}

	EndUndoAction();
}

void CScintilla::SetCode(wil::zstring_view code)
{
	SetText(code.data());
	SetSavePoint();
	ConvertEOLs(EndOfLine::CrLf);
	GrabFocus();
	TrackWidth();
}

void CScintilla::SetMode(ScintillaConfig::Mode mode)
{
	m_mode = mode;
	g_scintilla_config.set_mode(m_mode);
	SetStyles();
}

void CScintilla::SetIndentation(Line line, int indent)
{
	if (indent < 0) return;

	Range range = GetSelection();
	const Position pos_before = GetLineIndentPosition(line);
	SetLineIndentation(line, indent);
	const Position pos_after = GetLineIndentPosition(line);
	const Position pos_difference = pos_after - pos_before;
	if (pos_after > pos_before)
	{
		if (range.start >= pos_before)
		{
			range.start += pos_difference;
		}

		if (range.end >= pos_before)
		{
			range.end += pos_difference;
		}
	}
	else if (pos_after < pos_before)
	{
		if (range.start >= pos_after)
		{
			if (range.start >= pos_before)
				range.start += pos_difference;
			else
				range.start = pos_after;
		}

		if (range.end >= pos_after)
		{
			if (range.end >= pos_before)
				range.end += pos_difference;
			else
				range.end = pos_after;
		}
	}

	SetSel(range.start, range.end);
}

void CScintilla::SetStyle(wil::zstring_view name, wil::zstring_view value)
{
	if (value.empty()) return;

	const auto it = style_map.find(name);
	if (it != style_map.end())
	{
		EditorStyle style;

		for (auto&& str : js::split_string(value, ","))
		{
			const auto parts = js::split_string(str, ":");
			const std::string primary = parts[0];
			const std::string secondary = parts.size() == 2 ? parts[1] : "";

			if (primary == "font") style.font = secondary;
			else if (primary == "size" && pfc::string_is_numeric(secondary.c_str())) style.size = std::stoi(secondary);
			else if (primary == "fore") style.fore = ParseHex(secondary);
			else if (primary == "back") style.back = ParseHex(secondary);
			else if (primary == "bold") style.bold = true;
			else if (primary == "italics") style.italic = true;
		}

		for (const int id : it->second)
		{
			if (style.font.length()) StyleSetFont(id, style.font.c_str());
			if (style.size > 0) StyleSetSize(id, style.size);
			if (style.fore) StyleSetFore(id, *style.fore);
			if (style.back) StyleSetBack(id, *style.back);
			if (style.bold) StyleSetBold(id, style.bold);
			if (style.italic) StyleSetItalic(id, style.italic);
			StyleSetCheckMonospaced(id, true);
			if (id == STYLE_DEFAULT) StyleClearAll();
		}
	}
	else
	{
		const auto colour = ParseHex(value, true);
		if (colour)
		{
			if (name == "style.caret.fore")
			{
				SetElementColour(Element::Caret, *colour);
			}
			else if (name == "style.selection.back")
			{
				SetSelectionLayer(Layer::UnderText);
				SetElementColour(Element::SelectionBack, *colour);
				SetElementColour(Element::SelectionAdditionalBack, *colour);
				SetElementColour(Element::SelectionSecondaryBack, *colour);
				SetElementColour(Element::SelectionInactiveBack, *colour);
			}
		}
	}
}

void CScintilla::SetStyles()
{
	Utils::set_window_theme(m_hWnd, m_is_dark);

	ClearDocumentStyle();
	StyleResetDefault();

	auto data = GetConfigData();
	for (const auto& [name, value] : data)
	{
		SetStyle(name, value);
	}

	StyleSetFont(STYLE_LINENUMBER, "Consolas");
	StyleSetSize(STYLE_LINENUMBER, StyleGetSize(STYLE_DEFAULT) - 2);
	StyleSetBack(STYLE_LINENUMBER, GetSysColour(COLOR_BTNFACE));
	StyleSetFore(STYLE_LINENUMBER, GetSysColour(COLOR_BTNTEXT));

	SetElementColour(Element::List, GetSysColourAlpha(COLOR_WINDOWTEXT));
	SetElementColour(Element::ListBack, GetSysColourAlpha(COLOR_WINDOW));
	SetElementColour(Element::ListSelected, GetSysColourAlpha(COLOR_HIGHLIGHTTEXT));
	SetElementColour(Element::ListSelectedBack, GetSysColourAlpha(COLOR_HIGHLIGHT));

	AutoMarginWidth();
	Colourise(0, -1);
}

void CScintilla::StartAutoComplete()
{
	const Position current = GetCaretInLine();
	const std::string text = GetCurLineText();
	const std::string word_start = GetWordStart(text, current);

	auto filter = [word_start](const API& item) { return item.text.starts_with(word_start); };
	auto transform = [](const API& item) { return item.text.substr(0, item.len); };
	auto words = m_apis | std::views::filter(filter) | std::views::transform(transform);
	const std::string str = fmt::format("{}", fmt::join(words, " "));

	if (str.length())
	{
		AutoCShow(word_start.length(), str.c_str());
	}
}

void CScintilla::StartCallTip()
{
	static constexpr int min_length = 4; // Date is the shortest word before opening brace
	const Position current = GetCaretInLine() - 1;
	if (current < min_length) return;

	const std::string text = GetCurLineText();
	const std::string current_calltip_word = GetWordStart(text, current);
	const size_t len = current_calltip_word.length();
	if (len < min_length) return;

	const auto it = std::ranges::find_if(m_apis, [current_calltip_word, len](const API& item) { return item.text.length() > len && current_calltip_word == item.text.substr(0, item.len); });
	if (it != m_apis.end())
	{
		m_function_definition = it->text;
		CallTipShow(GetCurrentPos() - len, m_function_definition.c_str());
		ContinueCallTip();
	}
}

void CScintilla::TrackWidth()
{
	auto transform = [this](const Line line) { return PointXFromPosition(GetLineEndPosition(line)); };
	auto view = std::views::iota(0, GetLineCount()) | std::views::transform(transform);
	SetScrollWidth(std::ranges::max(view));
}

void CScintilla::ui_colors_changed()
{
	m_is_dark = ui_config_manager::g_is_dark_mode();
	SetStyles();
}
