
use druid::{AppLauncher, Data, Env, EventCtx, Lens, lens, LocalizedString, UnitPoint, Widget, WidgetExt, WindowDesc};
use druid::commands::QUIT_APP;
use druid::widget::{Button, Checkbox, ControllerHost, Flex, Label, RadioGroup};

const WINDOW_TITLE: LocalizedString<AppState> = LocalizedString::new("Rare Updater");

#[derive(Clone, Copy, PartialEq, Data)]
enum Version {
    Stable,
    Git,
}

#[derive(Clone, Data, Lens)]
struct AppState {
    install_pypresence: bool,
    install_webview: bool,
    version: Version,
    info_text: String
}

impl AppState {
    fn set_info_text(&mut self, text: String){
        self.info_text = text;
    }
}

pub fn main() {
    // describe the main window
    let main_window = WindowDesc::new(build_root_widget)
        .title(WINDOW_TITLE)
        .window_size((400.0, 400.0));

    // create the initial app state
    let initial_state = AppState {
        install_webview: false,
        install_pypresence: false,
        version: Version::Stable,
        info_text: "Downloading whatever...".into()
    };

    // start the application
    AppLauncher::with_window(main_window)
        .launch(initial_state)
        .expect("Failed to launch application");
}

fn install_rare(){

}

fn build_root_widget() -> impl Widget<AppState> {
    let versions = [("Stable", Version::Stable), ("Git", Version::Git)];

    // a label that will determine its text based on the current app data.
    let title = Label::new("Rare installer").with_text_size(30.0);

    let pypresence_checkbox = Checkbox::new("PyPresence (To show running games on Discord)")
        .lens(AppState::install_pypresence);

    let webview_checkbox =
        Checkbox::new("PyWebview (For a easier login)").lens(AppState::install_webview);

    let radio = RadioGroup::new(versions.to_vec()).lens(AppState::version);

    let cancel_button = Button::new("Cancel").on_click(|ctx, _, _| {
        ctx.submit_command(QUIT_APP);
    });
    let install_button = Button::new("Install").
    on_click(|_ctx: &mut EventCtx, data: &mut AppState, _env: &Env| data.set_info_text("Lol".into()));;

    let info_text = Label::new(|data: &AppState, _env: &Env| {
            data.info_text.to_string()
    });

    let button_layout = Flex::row()
        .with_child(cancel_button)
        .with_child(install_button);

    let layout = Flex::column()
        .with_child(title)
        .with_child(pypresence_checkbox)
        .with_child(webview_checkbox)
        .with_child(radio)
        .with_child(info_text)
        .with_child(button_layout)
        .align_horizontal(UnitPoint::TOP_LEFT)
        .padding(5.0);
    return layout;
}
