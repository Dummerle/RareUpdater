#![windows_subsystem = "windows"]
extern crate core;

mod install;
mod config;

use std::thread;

use druid::{AppDelegate, AppLauncher, Command, DelegateCtx, Env, EventCtx, ExtEventSink, Handled, LocalizedString, Target, UnitPoint, Widget, WidgetExt, WindowDesc};
use druid::commands::QUIT_APP;
use druid::widget::{Button, Either, Flex, Label, ViewSwitcher};
use crate::config::Config;
use crate::install::{AppState, CurrentScreen, GitHubResponse, STARTUP_ERROR, STARTUP_READY, uninstall, UNINSTALL_FINISHED};

const WINDOW_TITLE: LocalizedString<AppState> = LocalizedString::new("Rare Updater");


pub fn main() {
    let main_window = WindowDesc::new(main_widget)
        .title(WINDOW_TITLE)
        .window_size((400.0, 400.0));

    let config = Config::read();
    // create the initial app state
    let initial_state = AppState::new(config.installed_version);

    // start the application
    let launcher = AppLauncher::with_window(main_window).delegate(Delegate {});
    let event_sink = launcher.get_external_handle();

    load_startup(event_sink);

    launcher.launch(initial_state).expect("Failed to launch");
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
        } else if cmd.get(install::INSTALLATION_FINISHED).is_some() {
            data.set_info_text("Finished".to_string());
            data.installing = false;

            data.current_screen = CurrentScreen::Installed;

            let latest_version = match data.latest_rare_version.clone() {
                None => {
                    data.error_string = "Can't get latest version".to_string();
                    data.current_screen = CurrentScreen::Error;
                    return Handled::Yes;
                }
                Some(x) => x
            };

            let config = Config {
                installed: true,
                installed_version: latest_version,
            };
            config.save();

            Handled::Yes
        } else if let Some(err) = cmd.get(install::ERROR) {
            data.set_info_text(err.to_string());
            Handled::Yes
        } else if let Some(gh_resp) = cmd.get(install::STARTUP_READY) {
            let dl_link = match gh_resp.get_windows_download_link() {
                Ok(dl_link) => dl_link,
                Err(err) => {
                    data.set_info_text(err);
                    data.current_screen = CurrentScreen::Error;
                    return Handled::Yes;
                }
            };
            let config = Config::read();
            let installed_version = config.installed_version.as_str();
            data.latest_rare_version = Some(gh_resp.tag_name.clone());
            data.installed_version = config.installed_version.clone();
            data.download_link = Some(dl_link);
            if installed_version == "" {
                data.current_screen = CurrentScreen::Install
            } else {
                data.current_screen = CurrentScreen::Installed
            }
            Handled::Yes
        } else if let Some(err) = cmd.get(install::STARTUP_ERROR) {
            data.current_screen = CurrentScreen::Error;
            data.set_error_string(err.to_string());
            Handled::Yes
        } else if cmd.get(UNINSTALL_FINISHED).is_some() {
            data.installing = false;
            match Config::remove_file() {
                Ok(_) => {
                    data.current_screen = CurrentScreen::Install;
                    data.installed_version = "".to_string();
                    Handled::Yes
                }
                Err(err) => {
                    data.current_screen = CurrentScreen::Error;
                    data.error_string = err.to_string();
                    Handled::Yes
                }
            }
        } else {
            Handled::No
        }
    }
}

fn load_startup(event_sink: ExtEventSink) {
    thread::spawn(move || {
        let git_info = match GitHubResponse::get() {
            Ok(resp) => resp,
            Err(err) => {
                event_sink.submit_command(STARTUP_ERROR, err, Target::Auto).expect("");
                return;
            }
        };
        event_sink.submit_command(STARTUP_READY, git_info, Target::Auto).expect("Error");
    });
}

fn main_widget() -> impl Widget<AppState> {
    let config = Config::read();

    if config.installed {}

    return ViewSwitcher::new(
        |data: &AppState, _env| data.current_screen.clone(),
        |selector, _data, _env| match selector {
            CurrentScreen::Install => {
                Box::new(build_root_widget())
            }
            CurrentScreen::Error => {
                Box::new(error_screen())
            }
            CurrentScreen::Installed => {
                Box::new(installed_screen())
            }
            CurrentScreen::Loading => {
                Box::new(loading_screen())
            }
        },
    );
}

fn installed_screen() -> impl Widget<AppState> {
    let title_label = Label::new("Rare installer").with_text_size(30.0);

    let update_row = Flex::row()
        .with_child(Label::new(
            |data: &AppState, _: &Env| {
                format!("Update available: {} -> {}",
                        data.installed_version,
                        data.latest_rare_version.as_ref().unwrap())
            }
        ))
        .with_child(
            Button::new("Update")
        );

    let uninstall_button = Either::new(|data: &AppState, _| {
        !data.installing
    },
                                       Button::new("Uninstall")
                                           .on_click(
                                               |ctx: &mut EventCtx, data: &mut AppState, _env: &Env| {
                                                   data.installing = true;
                                                   println!("Uninstall Rare");
                                                   uninstall(ctx.get_external_handle(), false)
                                               }
                                           ),
                                       Label::new("Uninstalling"),
    );

    let layout = Flex::column()
        .with_child(title_label)
        .with_child(uninstall_button);

    return layout;
}

fn loading_screen() -> impl Widget<AppState> {
    return Label::new("Loading...").center();
}

fn error_screen() -> impl Widget<AppState> {
    let title = Label::new("Oops, an error occurred").with_text_size(20.0);

    let error_text = Label::new(|data: &AppState, _: &Env| { data.error_string.to_string() });

    return Flex::column()
        .with_child(title)
        .with_child(error_text);
}

fn build_root_widget() -> impl Widget<AppState> {
    // let versions = [("Stable", Version::Stable), ("Git", Version::Git)];

    // a label that will determine its text based on the current app data.
    let title = Label::new("Rare installer").with_text_size(30.0);

    /* let pypresence_checkbox = Checkbox::new("PyPresence (To show running games on Discord)")
         .lens(AppState::install_pypresence);

     let webview_checkbox =
         Checkbox::new("PyWebview (For a easier login)").lens(AppState::install_webview);
 */
    //let radio = RadioGroup::new(versions.to_vec()).lens(AppState::version);

    let cancel_button = Button::new("Cancel").on_click(|ctx, _, _| {
        ctx.submit_command(QUIT_APP);
    });

    let install_button = Button::new("Install").
        on_click(|ctx: &mut EventCtx, data: &mut AppState, _env: &Env| {
            data.installing = true;
            let link = match data.get_download_link() {
                None => {
                    data.set_info_text("Can't find download link".to_string());
                    return;
                }
                Some(link) => link
            };
            install::install(ctx.get_external_handle(), false, link);
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
        //  .with_child(radio)
        .with_child(info_text)
        .with_child(button_layout)
        .align_horizontal(UnitPoint::TOP_LEFT)
        .padding(5.0);
    return layout;
}
