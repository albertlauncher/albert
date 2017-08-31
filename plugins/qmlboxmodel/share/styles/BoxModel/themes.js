
var settableProperties = [
    "radius",
    "background_color",
    "foreground_color",
    "highlight_color",
    "border_color",
    "border_size",
    "input_fontsize",
    "input_color",
    "selection_color",
    "item_title_fontsize",
    "item_description_fontsize",
    "icon_size",
    "max_items",
    "font_name",
    "shadow_size",
    "shadow_color",
    "settingsbutton_size",
    "settingsbutton_color",
    "settingsbutton_hover_color",
    "spacing",
    "window_width"
];

function themes() {

    var themes = {}

    themes.Bright = {}
    themes.Bright.input_fontsize = 36
    themes.Bright.item_title_fontsize = 26
    themes.Bright.item_description_fontsize = 12
    themes.Bright.icon_size = 48
    themes.Bright.max_items = 5
    themes.Bright.spacing = 6
    themes.Bright.radius = 16
    themes.Bright.border_size = 6
    themes.Bright.settingsbutton_size = 16
    themes.Bright.window_width = 640
    themes.Bright.font_name = "Roboto"
    themes.Bright.background_color = "#FFFFFF"
    themes.Bright.foreground_color = "#a0a0a0"
    themes.Bright.highlight_color = "#606060"
    themes.Bright.border_color = "#606060"
    themes.Bright.shadow_color = "#30000000"
    themes.Bright.input_color = themes.Bright.foreground_color
    themes.Bright.selection_color = themes.Bright.highlight_color
    themes.Bright.settingsbutton_color = themes.Bright.background_color
    themes.Bright.settingsbutton_hover_color = themes.Bright.border_color

    themes.BrightOrange = JSON.parse(JSON.stringify(themes.Bright))
    themes.BrightOrange.highlight_color = "#FFc080"
    themes.BrightOrange.border_color = themes.BrightOrange.highlight_color
    themes.BrightOrange.input_color = themes.BrightOrange.foreground_color
    themes.BrightOrange.selection_color = themes.BrightOrange.highlight_color
    themes.BrightOrange.settingsbutton_color = themes.BrightOrange.background_color
    themes.BrightOrange.settingsbutton_hover_color = themes.BrightOrange.border_color

    themes.BrightMagenta = JSON.parse(JSON.stringify(themes.Bright))
    themes.BrightMagenta.highlight_color = "#FF80c0"
    themes.BrightMagenta.border_color =themes.BrightMagenta.highlight_color
    themes.BrightMagenta.input_color = themes.BrightMagenta.foreground_color
    themes.BrightMagenta.selection_color = themes.BrightMagenta.highlight_color
    themes.BrightMagenta.settingsbutton_color = themes.BrightMagenta.background_color
    themes.BrightMagenta.settingsbutton_hover_color = themes.BrightMagenta.border_color

    themes.BrightMint = JSON.parse(JSON.stringify(themes.Bright))
    themes.BrightMint.highlight_color = "#00FF80"
    themes.BrightMint.border_color =themes.BrightMint.highlight_color
    themes.BrightMint.input_color = themes.BrightMint.foreground_color
    themes.BrightMint.selection_color = themes.BrightMint.highlight_color
    themes.BrightMint.settingsbutton_color = themes.BrightMint.background_color
    themes.BrightMint.settingsbutton_hover_color = themes.BrightMint.border_color

    themes.BrightGreen = JSON.parse(JSON.stringify(themes.Bright))
    themes.BrightGreen.highlight_color = "#78f000"
    themes.BrightGreen.border_color = themes.BrightGreen.highlight_color
    themes.BrightGreen.input_color = themes.BrightGreen.foreground_color
    themes.BrightGreen.selection_color = themes.BrightGreen.highlight_color
    themes.BrightGreen.settingsbutton_color = themes.BrightGreen.background_color
    themes.BrightGreen.settingsbutton_hover_color = themes.BrightGreen.border_color

    themes.BrightBlue = JSON.parse(JSON.stringify(themes.Bright))
    themes.BrightBlue.highlight_color = "#80c0ff"
    themes.BrightBlue.border_color = themes.BrightBlue.highlight_color
    themes.BrightBlue.input_color = themes.BrightBlue.foreground_color
    themes.BrightBlue.selection_color = themes.BrightBlue.highlight_color
    themes.BrightBlue.settingsbutton_color = themes.BrightBlue.background_color
    themes.BrightBlue.settingsbutton_hover_color = themes.BrightBlue.border_color

    themes.BrightViolet = JSON.parse(JSON.stringify(themes.Bright))
    themes.BrightViolet.highlight_color = "#C080FF"
    themes.BrightViolet.border_color =themes.BrightViolet.highlight_color
    themes.BrightViolet.input_color = themes.BrightViolet.foreground_color
    themes.BrightViolet.selection_color = themes.BrightViolet.highlight_color
    themes.BrightViolet.settingsbutton_color = themes.BrightViolet.background_color
    themes.BrightViolet.settingsbutton_hover_color = themes.BrightViolet.border_color

    themes.Dark = JSON.parse(JSON.stringify(themes.Bright))
    themes.Dark.background_color = "#404040"
    themes.Dark.foreground_color = "#808080"
    themes.Dark.highlight_color = "#E0E0E0"
    themes.Dark.border_color = "#808080"
    themes.Dark.input_color = themes.Dark.foreground_color
    themes.Dark.selection_color = themes.Dark.highlight_color
    themes.Dark.settingsbutton_color = themes.Dark.background_color
    themes.Dark.settingsbutton_hover_color = themes.Dark.border_color

    themes.DarkOrange = JSON.parse(JSON.stringify(themes.Dark))
    themes.DarkOrange.highlight_color = "#FF9020"
    themes.DarkOrange.border_color = "#FF8000"
    themes.DarkOrange.input_color = themes.DarkOrange.foreground_color
    themes.DarkOrange.selection_color = themes.DarkOrange.highlight_color
    themes.DarkOrange.settingsbutton_color = themes.DarkOrange.background_color
    themes.DarkOrange.settingsbutton_hover_color = themes.DarkOrange.border_color

    themes.DarkRed = JSON.parse(JSON.stringify(themes.Dark))
    themes.DarkRed.highlight_color = "#FF9020"
    themes.DarkRed.border_color = "#FF8000"
    themes.DarkRed.input_color = themes.DarkRed.foreground_color
    themes.DarkRed.selection_color = themes.DarkRed.highlight_color
    themes.DarkRed.settingsbutton_color = themes.DarkRed.background_color
    themes.DarkRed.settingsbutton_hover_color = themes.DarkRed.border_color

    themes.DarkMagenta = JSON.parse(JSON.stringify(themes.Dark))
    themes.DarkMagenta.highlight_color = "#FF2090"
    themes.DarkMagenta.border_color = "#FF0080"
    themes.DarkMagenta.input_color = themes.DarkMagenta.foreground_color
    themes.DarkMagenta.selection_color = themes.DarkMagenta.highlight_color
    themes.DarkMagenta.settingsbutton_color = themes.DarkMagenta.background_color
    themes.DarkMagenta.settingsbutton_hover_color = themes.DarkMagenta.border_color

    themes.DarkMint = JSON.parse(JSON.stringify(themes.Dark))
    themes.DarkMint.highlight_color = "#20FF90"
    themes.DarkMint.border_color = "#00FF80"
    themes.DarkMint.input_color = themes.DarkMint.foreground_color
    themes.DarkMint.selection_color = themes.DarkMint.highlight_color
    themes.DarkMint.settingsbutton_color = themes.DarkMint.background_color
    themes.DarkMint.settingsbutton_hover_color = themes.DarkMint.border_color

    themes.DarkGreen = JSON.parse(JSON.stringify(themes.Dark))
    themes.DarkGreen.highlight_color = "#90FF20"
    themes.DarkGreen.border_color = "#80FF00"
    themes.DarkGreen.input_color = themes.DarkGreen.foreground_color
    themes.DarkGreen.selection_color = themes.DarkGreen.highlight_color
    themes.DarkGreen.settingsbutton_color = themes.DarkGreen.background_color
    themes.DarkGreen.settingsbutton_hover_color = themes.DarkGreen.border_color

    themes.DarkBlue = JSON.parse(JSON.stringify(themes.Dark))
    themes.DarkBlue.highlight_color = "#2090FF"
    themes.DarkBlue.border_color = "#0080FF"
    themes.DarkBlue.input_color = themes.DarkBlue.foreground_color
    themes.DarkBlue.selection_color = themes.DarkBlue.highlight_color
    themes.DarkBlue.settingsbutton_color = themes.DarkBlue.background_color
    themes.DarkBlue.settingsbutton_hover_color = themes.DarkBlue.border_color

    themes.DarkViolet = JSON.parse(JSON.stringify(themes.Dark))
    themes.DarkViolet.highlight_color = "#A040FF"
    themes.DarkViolet.border_color = "#8000FF"
    themes.DarkViolet.input_color = themes.DarkViolet.foreground_color
    themes.DarkViolet.selection_color = themes.DarkViolet.highlight_color
    themes.DarkViolet.settingsbutton_color = themes.DarkViolet.background_color
    themes.DarkViolet.settingsbutton_hover_color = themes.DarkViolet.border_color

    themes.SolarizedBrightYellow = JSON.parse(JSON.stringify(themes.Dark))
    themes.SolarizedBrightYellow.background_color = "#fdf6e3"
    themes.SolarizedBrightYellow.foreground_color = "#839496"
    themes.SolarizedBrightYellow.highlight_color = "#b58900"
    themes.SolarizedBrightYellow.border_color = "#b58900"
    themes.SolarizedBrightYellow.input_color = themes.SolarizedBrightYellow.foreground_color
    themes.SolarizedBrightYellow.selection_color = themes.SolarizedBrightYellow.highlight_color
    themes.SolarizedBrightYellow.settingsbutton_color = themes.SolarizedBrightYellow.background_color
    themes.SolarizedBrightYellow.settingsbutton_hover_color = themes.SolarizedBrightYellow.border_color

    themes.SolarizedBrightOrange = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedBrightOrange.highlight_color = "#cb4b16"
    themes.SolarizedBrightOrange.border_color = "#cb4b16"
    themes.SolarizedBrightOrange.input_color = themes.SolarizedBrightOrange.foreground_color
    themes.SolarizedBrightOrange.selection_color = themes.SolarizedBrightOrange.highlight_color
    themes.SolarizedBrightOrange.settingsbutton_color = themes.SolarizedBrightOrange.background_color
    themes.SolarizedBrightOrange.settingsbutton_hover_color = themes.SolarizedBrightOrange.border_color

    themes.SolarizedBrightRed = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedBrightRed.highlight_color = "#dc322f"
    themes.SolarizedBrightRed.border_color = "#dc322f"
    themes.SolarizedBrightRed.input_color = themes.SolarizedBrightRed.foreground_color
    themes.SolarizedBrightRed.selection_color = themes.SolarizedBrightRed.highlight_color
    themes.SolarizedBrightRed.settingsbutton_color = themes.SolarizedBrightRed.background_color
    themes.SolarizedBrightRed.settingsbutton_hover_color = themes.SolarizedBrightRed.border_color

    themes.SolarizedBrightMagenta = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedBrightMagenta.highlight_color = "#d33682"
    themes.SolarizedBrightMagenta.border_color = "#d33682"
    themes.SolarizedBrightMagenta.input_color = themes.SolarizedBrightMagenta.foreground_color
    themes.SolarizedBrightMagenta.selection_color = themes.SolarizedBrightMagenta.highlight_color
    themes.SolarizedBrightMagenta.settingsbutton_color = themes.SolarizedBrightMagenta.background_color
    themes.SolarizedBrightMagenta.settingsbutton_hover_color = themes.SolarizedBrightMagenta.border_color

    themes.SolarizedBrightViolet = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedBrightViolet.highlight_color = "#6c71c4"
    themes.SolarizedBrightViolet.border_color = "#6c71c4"
    themes.SolarizedBrightViolet.input_color = themes.SolarizedBrightViolet.foreground_color
    themes.SolarizedBrightViolet.selection_color = themes.SolarizedBrightViolet.highlight_color
    themes.SolarizedBrightViolet.settingsbutton_color = themes.SolarizedBrightViolet.background_color
    themes.SolarizedBrightViolet.settingsbutton_hover_color = themes.SolarizedBrightViolet.border_color

    themes.SolarizedBrightBlue = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedBrightBlue.highlight_color = "#268bd2"
    themes.SolarizedBrightBlue.border_color = "#268bd2"
    themes.SolarizedBrightBlue.input_color = themes.SolarizedBrightBlue.foreground_color
    themes.SolarizedBrightBlue.selection_color = themes.SolarizedBrightBlue.highlight_color
    themes.SolarizedBrightBlue.settingsbutton_color = themes.SolarizedBrightBlue.background_color
    themes.SolarizedBrightBlue.settingsbutton_hover_color = themes.SolarizedBrightBlue.border_color

    themes.SolarizedBrightCyan = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedBrightCyan.highlight_color = "#2aa198"
    themes.SolarizedBrightCyan.border_color = "#2aa198"
    themes.SolarizedBrightCyan.input_color = themes.SolarizedBrightCyan.foreground_color
    themes.SolarizedBrightCyan.selection_color = themes.SolarizedBrightCyan.highlight_color
    themes.SolarizedBrightCyan.settingsbutton_color = themes.SolarizedBrightCyan.background_color
    themes.SolarizedBrightCyan.settingsbutton_hover_color = themes.SolarizedBrightCyan.border_color

    themes.SolarizedBrightGreen = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedBrightGreen.highlight_color = "#859900"
    themes.SolarizedBrightGreen.border_color = "#859900"
    themes.SolarizedBrightGreen.input_color = themes.SolarizedBrightGreen.foreground_color
    themes.SolarizedBrightGreen.selection_color = themes.SolarizedBrightGreen.highlight_color
    themes.SolarizedBrightGreen.settingsbutton_color = themes.SolarizedBrightGreen.background_color
    themes.SolarizedBrightGreen.settingsbutton_hover_color = themes.SolarizedBrightGreen.border_color

    themes.SolarizedDarkYellow = JSON.parse(JSON.stringify(themes.SolarizedBrightYellow))
    themes.SolarizedDarkYellow.background_color = "#002b36"
    themes.SolarizedDarkYellow.highlight_color = "#b58900"
    themes.SolarizedDarkYellow.border_color = "#b58900"
    themes.SolarizedDarkYellow.input_color = themes.SolarizedDarkYellow.foreground_color
    themes.SolarizedDarkYellow.selection_color = themes.SolarizedDarkYellow.highlight_color
    themes.SolarizedDarkYellow.settingsbutton_color = themes.SolarizedDarkYellow.background_color
    themes.SolarizedDarkYellow.settingsbutton_hover_color = themes.SolarizedDarkYellow.border_color

    themes.SolarizedDarkOrange = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.SolarizedDarkOrange.highlight_color = "#cb4b16"
    themes.SolarizedDarkOrange.border_color = "#cb4b16"
    themes.SolarizedDarkOrange.input_color = themes.SolarizedDarkOrange.foreground_color
    themes.SolarizedDarkOrange.selection_color = themes.SolarizedDarkOrange.highlight_color
    themes.SolarizedDarkOrange.settingsbutton_color = themes.SolarizedDarkOrange.background_color
    themes.SolarizedDarkOrange.settingsbutton_hover_color = themes.SolarizedDarkOrange.border_color

    themes.SolarizedDarkRed = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.SolarizedDarkRed.highlight_color = "#dc322f"
    themes.SolarizedDarkRed.border_color = "#dc322f"
    themes.SolarizedDarkRed.input_color = themes.SolarizedDarkRed.foreground_color
    themes.SolarizedDarkRed.selection_color = themes.SolarizedDarkRed.highlight_color
    themes.SolarizedDarkRed.settingsbutton_color = themes.SolarizedDarkRed.background_color
    themes.SolarizedDarkRed.settingsbutton_hover_color = themes.SolarizedDarkRed.border_color

    themes.SolarizedDarkMagenta = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.SolarizedDarkMagenta.highlight_color = "#d33682"
    themes.SolarizedDarkMagenta.border_color = "#d33682"
    themes.SolarizedDarkMagenta.input_color = themes.SolarizedDarkMagenta.foreground_color
    themes.SolarizedDarkMagenta.selection_color = themes.SolarizedDarkMagenta.highlight_color
    themes.SolarizedDarkMagenta.settingsbutton_color = themes.SolarizedDarkMagenta.background_color
    themes.SolarizedDarkMagenta.settingsbutton_hover_color = themes.SolarizedDarkMagenta.border_color

    themes.SolarizedDarkViolet = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.SolarizedDarkViolet.highlight_color = "#6c71c4"
    themes.SolarizedDarkViolet.border_color = "#6c71c4"
    themes.SolarizedDarkViolet.input_color = themes.SolarizedDarkViolet.foreground_color
    themes.SolarizedDarkViolet.selection_color = themes.SolarizedDarkViolet.highlight_color
    themes.SolarizedDarkViolet.settingsbutton_color = themes.SolarizedDarkViolet.background_color
    themes.SolarizedDarkViolet.settingsbutton_hover_color = themes.SolarizedDarkViolet.border_color

    themes.SolarizedDarkBlue = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.SolarizedDarkBlue.highlight_color = "#268bd2"
    themes.SolarizedDarkBlue.border_color = "#268bd2"
    themes.SolarizedDarkBlue.input_color = themes.SolarizedDarkBlue.foreground_color
    themes.SolarizedDarkBlue.selection_color = themes.SolarizedDarkBlue.highlight_color
    themes.SolarizedDarkBlue.settingsbutton_color = themes.SolarizedDarkBlue.background_color
    themes.SolarizedDarkBlue.settingsbutton_hover_color = themes.SolarizedDarkBlue.border_color

    themes.SolarizedDarkCyan = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.SolarizedDarkCyan.highlight_color = "#2aa198"
    themes.SolarizedDarkCyan.border_color = "#2aa198"
    themes.SolarizedDarkCyan.input_color = themes.SolarizedDarkCyan.foreground_color
    themes.SolarizedDarkCyan.selection_color = themes.SolarizedDarkCyan.highlight_color
    themes.SolarizedDarkCyan.settingsbutton_color = themes.SolarizedDarkCyan.background_color
    themes.SolarizedDarkCyan.settingsbutton_hover_color = themes.SolarizedDarkCyan.border_color

    themes.SolarizedDarkGreen = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.SolarizedDarkGreen.highlight_color = "#859900"
    themes.SolarizedDarkGreen.border_color = "#859900"
    themes.SolarizedDarkGreen.input_color = themes.SolarizedDarkGreen.foreground_color
    themes.SolarizedDarkGreen.selection_color = themes.SolarizedDarkGreen.highlight_color
    themes.SolarizedDarkGreen.settingsbutton_color = themes.SolarizedDarkGreen.background_color
    themes.SolarizedDarkGreen.settingsbutton_hover_color = themes.SolarizedDarkGreen.border_color

    themes.Tiffany = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.Tiffany.background_color = "#e2f2fa"
    themes.Tiffany.foreground_color = "#73BDE4"
    themes.Tiffany.highlight_color = "#a47a51"
    themes.Tiffany.border_color = "#1d6a87"
    themes.Tiffany.input_color = themes.Tiffany.foreground_color
    themes.Tiffany.selection_color = themes.Tiffany.highlight_color
    themes.Tiffany.settingsbutton_color = themes.Tiffany.background_color
    themes.Tiffany.settingsbutton_hover_color = themes.Tiffany.border_color

    themes.Nerdy = JSON.parse(JSON.stringify(themes.SolarizedDarkYellow))
    themes.Nerdy.input_fontsize = 22
    themes.Nerdy.item_title_fontsize = 16
    themes.Nerdy.item_description_fontsize = 12
    themes.Nerdy.icon_size = 32
    themes.Nerdy.max_items = 10
    themes.Nerdy.spacing = 3
    themes.Nerdy.radius = 8
    themes.Nerdy.border_size = 1
    themes.Nerdy.settingsbutton_size = 14
    themes.Nerdy.window_width = 600
    themes.Nerdy.font_name = "monospace"
    themes.Nerdy.background_color = "#202020"
    themes.Nerdy.foreground_color = "#808080"
    themes.Nerdy.highlight_color = "#00FF00"
    themes.Nerdy.border_color = "#404040"
    themes.Nerdy.input_color = themes.Nerdy.highlight_color
    themes.Nerdy.selection_color = themes.Nerdy.foreground_color
    themes.Nerdy.settingsbutton_color = "#404040"
    themes.Nerdy.settingsbutton_hover_color = themes.Nerdy.highlight_color

    return themes
}
