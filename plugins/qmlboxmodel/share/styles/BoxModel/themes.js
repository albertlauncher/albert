
function themes() {

    var themes = {}

    themes.Bright = {}
    themes.Bright.input_fontsize = 36
    themes.Bright.item_title_fontsize = 26
    themes.Bright.item_description_fontsize = 12
    themes.Bright.icon_size = 48
    themes.Bright.max_items = 5
    themes.Bright.spacing = 6
    themes.Bright.padding = 6
    themes.Bright.radius = 16
    themes.Bright.border_size = 6
    themes.Bright.settingsbutton_size = 16
    themes.Bright.animation_duration = 200
    themes.Bright.window_width = 640
    themes.Bright.font_name = "Roboto"
    themes.Bright.shadow_size = 30
    themes.Bright.shadow_color               = "#40000000"
    themes.Bright.foreground_color           = "#a0a0a0"
    themes.Bright.background_color           = "#FFFFFF"
    themes.Bright.highlight_color            = "#606060"
    themes.Bright.border_color               = themes.Bright.foreground_color
    themes.Bright.input_color                = themes.Bright.foreground_color
    themes.Bright.cursor_color               = themes.Bright.foreground_color
    themes.Bright.selection_color            = themes.Bright.highlight_color
    themes.Bright.settingsbutton_hover_color = themes.Bright.highlight_color
    themes.Bright.settingsbutton_color       = themes.Bright.background_color

    var highlights = {}
    highlights.BrightOrange  = "#ff9f3f"
    highlights.BrightMagenta = "#ff3f9f"
    highlights.BrightMint    = "#3fff9f"
    highlights.BrightGreen   = "#9fff3f"
    highlights.BrightBlue    = "#3f9fff"
    highlights.BrightViolet  = "#9f3fff"

    for (var name in highlights){
        if (highlights.hasOwnProperty(name)){
            themes[name] = JSON.parse(JSON.stringify(themes.Bright))
            themes[name].highlight_color            = highlights[name]
            themes[name].selection_color            = highlights[name]
            themes[name].border_color               = highlights[name]
            themes[name].settingsbutton_hover_color = highlights[name]
        }
    }

    themes.Dark = JSON.parse(JSON.stringify(themes.Bright))
    themes.Dark.foreground_color           = "#808080"
    themes.Dark.background_color           = "#404040"
    themes.Dark.highlight_color            = "#E0E0E0"
    themes.Dark.border_color               = themes.Dark.foreground_color
    themes.Dark.input_color                = themes.Dark.foreground_color
    themes.Dark.cursor_color               = themes.Dark.foreground_color
    themes.Dark.settingsbutton_color       = themes.Dark.background_color
    themes.Dark.settingsbutton_hover_color = themes.Dark.highlight_color
    themes.Dark.selection_color            = themes.Dark.highlight_color

    highlights = {}
    highlights.DarkOrange  = "#FF9020"
    highlights.DarkMagenta = "#FF2090"
    highlights.DarkMint    = "#20FF90"
    highlights.DarkGreen   = "#90FF20"
    highlights.DarkBlue    = "#2090FF"
    highlights.DarkViolet  = "#9020FF"

    for (name in highlights){
        if (highlights.hasOwnProperty(name)){
            themes[name] = JSON.parse(JSON.stringify(themes.Dark))
            themes[name].highlight_color            = highlights[name]
            themes[name].selection_color            = highlights[name]
            themes[name].border_color               = highlights[name]
            themes[name].settingsbutton_hover_color = highlights[name]
        }
    }

    themes.SolarizedBrightYellow = JSON.parse(JSON.stringify(themes.Dark))
    themes.SolarizedBrightYellow.background_color = "#fdf6e3"
    themes.SolarizedBrightYellow.foreground_color = "#839496"
    themes.SolarizedBrightYellow.highlight_color  = "#b58900"
    themes.SolarizedBrightYellow.input_color                = themes.SolarizedBrightYellow.foreground_color
    themes.SolarizedBrightYellow.cursor_color               = themes.SolarizedBrightYellow.foreground_color
    themes.SolarizedBrightYellow.settingsbutton_color       = themes.SolarizedBrightYellow.background_color
    themes.SolarizedBrightYellow.settingsbutton_hover_color = themes.SolarizedBrightYellow.highlight_color
    themes.SolarizedBrightYellow.selection_color            = themes.SolarizedBrightYellow.highlight_color
    themes.SolarizedBrightYellow.border_color               = themes.SolarizedBrightYellow.highlight_color

    highlights = {}
    highlights.SolarizedBrightOrange  = "#cb4b16"
    highlights.SolarizedBrightRed     = "#dc322f"
    highlights.SolarizedBrightMagenta = "#d33682"
    highlights.SolarizedBrightCyan    = "#2aa198"
    highlights.SolarizedBrightViolet  = "#6c71c4"
    highlights.SolarizedBrightBlue    = "#268bd2"
    highlights.SolarizedBrightGreen   = "#859900"

    for (name in highlights){
        if (highlights.hasOwnProperty(name)){
            themes[name] = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
            themes[name].highlight_color            = highlights[name]
            themes[name].selection_color            = highlights[name]
            themes[name].border_color               = highlights[name]
            themes[name].settingsbutton_hover_color = highlights[name]
        }
    }

    themes.SolarizedDarkYellow = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedDarkYellow.background_color = "#002b36"
    themes.SolarizedDarkYellow.highlight_color  = "#b58900"
    themes.SolarizedDarkYellow.input_color                = themes.SolarizedDarkYellow.foreground_color
    themes.SolarizedDarkYellow.cursor_color               = themes.SolarizedDarkYellow.foreground_color
    themes.SolarizedDarkYellow.settingsbutton_color       = themes.SolarizedDarkYellow.background_color
    themes.SolarizedDarkYellow.settingsbutton_hover_color = themes.SolarizedDarkYellow.highlight_color
    themes.SolarizedDarkYellow.selection_color            = themes.SolarizedDarkYellow.highlight_color
    themes.SolarizedDarkYellow.border_color               = themes.SolarizedDarkYellow.highlight_color

    highlights = {}
    highlights.SolarizedDarkOrange  = "#cb4b16"
    highlights.SolarizedDarkRed     = "#dc322f"
    highlights.SolarizedDarkMagenta = "#d33682"
    highlights.SolarizedDarkCyan    = "#2aa198"
    highlights.SolarizedDarkViolet  = "#6c71c4"
    highlights.SolarizedDarkBlue    = "#268bd2"
    highlights.SolarizedDarkGreen   = "#859900"

    for (name in highlights){
        if (highlights.hasOwnProperty(name)){
            themes[name] = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
            themes[name].highlight_color            = highlights[name]
            themes[name].selection_color            = highlights[name]
            themes[name].border_color               = highlights[name]
            themes[name].settingsbutton_hover_color = highlights[name]
        }
    }

    themes.Tiffany = JSON.parse(JSON.stringify(themes.Bright))
    themes.Tiffany.background_color = "#e2f2fa"
    themes.Tiffany.foreground_color = "#73BDE4"
    themes.Tiffany.highlight_color = "#a47a51"
    themes.Tiffany.border_color = "#1d6a87"
    themes.Tiffany.input_color                = themes.Tiffany.foreground_color
    themes.Tiffany.cursor_color               = themes.Tiffany.foreground_color
    themes.Tiffany.settingsbutton_color       = themes.Tiffany.background_color
    themes.Tiffany.settingsbutton_hover_color = themes.Tiffany.highlight_color
    themes.Tiffany.selection_color            = themes.Tiffany.highlight_color

    themes.Nerdy = JSON.parse(JSON.stringify(themes.Bright))
    themes.Nerdy.icon_size = 36
    themes.Nerdy.input_fontsize = 26
    themes.Nerdy.item_title_fontsize = 18
    themes.Nerdy.item_description_fontsize = 11
    themes.Nerdy.max_items = 8
    themes.Nerdy.spacing = 6
    themes.Nerdy.radius = 6
    themes.Nerdy.border_size = 1
    themes.Nerdy.settingsbutton_size = 14
    themes.Nerdy.window_width = 600
    themes.Nerdy.font_name = "monospace"
    themes.Nerdy.background_color = "#202020"
    themes.Nerdy.foreground_color = "#808080"
    themes.Nerdy.highlight_color = "#00FF00"
    themes.Nerdy.border_color = "#404040"
    themes.Nerdy.cursor_color               = themes.Nerdy.highlight_color
    themes.Nerdy.input_color                = themes.Nerdy.highlight_color
    themes.Nerdy.selection_color            = themes.Nerdy.foreground_color
    themes.Nerdy.settingsbutton_color       = "#303030"
    themes.Nerdy.settingsbutton_hover_color = themes.Nerdy.highlight_color

    return themes
}
