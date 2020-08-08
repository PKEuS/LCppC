#include "CppField.h"


CppField::CppField(wxWindow* parent_, wxWindowID id)
    : wxStyledTextCtrl(parent_, id)
{
    SetLexer(wxSTC_LEX_CPP);

    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(75, 75, 75));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(220, 220, 220));
    SetMarginType(0, wxSTC_MARGIN_NUMBER);
    wxFont font(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    for (size_t n = 0; n < wxSTC_STYLE_MAX; n++) {
        StyleSetFont(n, font);
    }

    SetTabWidth(4);

    StyleSetForeground(wxSTC_C_STRING, wxColour(150, 0, 0));
    StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(165, 105, 0));
    StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(40, 0, 60));
    StyleSetForeground(wxSTC_C_NUMBER, wxColour(0, 150, 0));
    StyleSetForeground(wxSTC_C_CHARACTER, wxColour(150, 0, 0));
    StyleSetForeground(wxSTC_C_WORD, wxColour(0, 0, 150));
    StyleSetForeground(wxSTC_C_WORD2, wxColour(150, 0, 150));
    StyleSetForeground(wxSTC_C_COMMENT, wxColour(150, 150, 150));
    StyleSetForeground(wxSTC_C_COMMENTLINE, wxColour(150, 150, 150));
    StyleSetForeground(wxSTC_C_COMMENTDOC, wxColour(150, 150, 150));
    StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORD, wxColour(0, 0, 200));
    StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORDERROR, wxColour(0, 0, 200));
    StyleSetBold(wxSTC_C_WORD, true);
    StyleSetBold(wxSTC_C_WORD2, true);
    StyleSetBold(wxSTC_C_COMMENTDOCKEYWORD, true);

    SetKeyWords(0, wxT("alignas alignof auto bool char char8_t char16_t char32_t class concept const consteval constexpr constinit const_cast "
                       "decltype delete double dynamic_cast  enum explicit export extern false float friend inline int long mutable namespace new "
                       "noexcept nullptr operator private protected public register reinterpret_cast requires short signed sizeof static "
                       "static_assert static_cast struct template this thread_local true typedef typeid typename union unsigned using virtual "
                       "void volatile wchar_t"));
    SetKeyWords(1, wxT("break case catch continue co_await co_return co_yield default do else switch for goto if return throw try while"));

    SetScrollWidth(1);
    SetScrollWidthTracking(true);
    SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, "100 "));
}
