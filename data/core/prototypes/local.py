import jactorioData as j

(j.Localization()
    .identifier("en")
    .fontPath("core/font/NotoSans-Regular.ttf")
    .fontSize(16)
)

# Labels

for i in range(13):
    j.Label("label:player-action-" + str(i))

j.Label("label:key-action-none")
j.Label("label:key-action-key-down")
j.Label("label:key-action-key-pressed")
j.Label("label:key-action-key-repeat")
j.Label("label:key-action-key-held")
j.Label("label:key-action-key-up")    

j.Label("label:key-mouse-left")
j.Label("label:key-mouse-middle")
j.Label("label:key-mouse-right")
j.Label("label:key-mouse-x1")
j.Label("label:key-mouse-x2")

j.Label("label:menu-main")
j.Label("label:menu-new-game")
j.Label("label:menu-new-game-seed")
j.Label("label:menu-new-game-play")
j.Label("label:menu-load-game")
j.Label("label:menu-load-game-play")
j.Label("label:menu-save-game")
j.Label("label:menu-save-game-invalid-name")
j.Label("label:menu-save-game-save-name")
j.Label("label:menu-save-game-save")
j.Label("label:menu-options")
j.Label("label:menu-option-change-keybind")
j.Label("label:menu-option-change-keybind-key")
j.Label("label:menu-option-change-keybind-name")
j.Label("label:menu-option-change-keybind-action")
j.Label("label:menu-option-change-keybind-reset")
j.Label("label:menu-option-toggle-fullscreen")
j.Label("label:menu-quit")
