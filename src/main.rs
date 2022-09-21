extern crate core;

mod install;

use install::{AppState, Version};

use druid::{AppDelegate, AppLauncher, Command, DelegateCtx, Env, EventCtx, Handled, LocalizedString, Target, UnitPoint, Widget, WidgetExt, WindowDesc};
use druid::commands::QUIT_APP;
use druid::widget::{Button, Either, Flex, Label, RadioGroup};

const WINDOW_TITLE: LocalizedString<AppState> = LocalizedString::new("Rare Updater");


pub fn main() {
    // describe the main window
    let main_window = WindowDesc::new(build_root_widget)
        .title(WINDOW_TITLE)
        .window_size((400.0, 400.0));

    // create the initial app state
    let initial_state = AppState::default();

    // start the application
    AppLauncher::with_window(main_window)
        .delegate(Delegate {})
        .launch(initial_state)
        .expect("Failed to launch application");
}

struct Delegate;

impl AppDelegate<AppState> for Delegate {
    fn command(&mut self, _ctx: &mut DelegateCtx,
               _target: Target, cmd: &Command, data: &mut AppState,
               _env: &Env, ) -> Handled {
        if let Some(text) = cmd.get(install::STATE_UPDATE) {
            // If the command we received is `FINISH_SLOW_FUNCTION` handle the payload.
            data.set_info_text(text.to_string());
            Handled::Yes
        } else if cmd.get(install::FINISHED).is_some() {
            data.set_info_text("Finished".to_string());
            Handled::Yes
        }

        else {
            Handled::No
        }
    }
}


fn build_root_widget() -> impl Widget<AppState> {
    let versions = [("Stable", Version::Stable), ("Git", Version::Git)];

    // a label that will determine its text based on the current app data.
    let title = Label::new("Rare installer").with_text_size(30.0);

   /* let pypresence_checkbox = Checkbox::new("PyPresence (To show running games on Discord)")
        .lens(AppState::install_pypresence);

    let webview_checkbox =
        Checkbox::new("PyWebview (For a easier login)").lens(AppState::install_webview);
*/
    let radio = RadioGroup::new(versions.to_vec()).lens(AppState::version);

    let cancel_button = Button::new("Cancel").on_click(|ctx, _, _| {
        ctx.submit_command(QUIT_APP);
    });

    let install_button = Button::new("Install").
        on_click(|ctx: &mut EventCtx, data: &mut AppState, _env: &Env| {
            data.installing = true;
            let _ = install::install(ctx.get_external_handle(), data.get_install_options());
        });

    let info_text = Label::new(|data: &AppState, _env: &Env| {
        data.info_text.to_string()
    });

    let button_layout = Either::new(
        |data: &AppState, _env| !data.installing,
        Flex::row()
            .with_child(cancel_button)
            .with_child(install_button),
        Label::new("Installing"),
    );

    let layout = Flex::column()
        .with_child(title)
       // .with_child(pypresence_checkbox)
       // .with_child(webview_checkbox)
        .with_child(radio)
        .with_child(info_text)
        .with_child(button_layout)
        .align_horizontal(UnitPoint::TOP_LEFT)
        .padding(5.0);
    return layout;
}
