#![windows_subsystem = "windows"]
extern crate core;

mod install;
mod config;

use std::thread;
use dirs::data_local_dir;
use subprocess::{Popen, PopenConfig};

use druid::{AppDelegate, AppLauncher, Command, DelegateCtx, Env, EventCtx, ExtEventSink, Handled, LocalizedString, Target, Widget, WidgetExt, WindowDesc};
use druid::commands::QUIT_APP;
use druid::widget::{Button, Either, Flex, Label, LineBreaking, Padding, ViewSwitcher};
use crate::config::Config;
use crate::install::{AppState, CurrentScreen, ERROR, GitHubResponse, install, STARTUP_ERROR, STARTUP_READY, uninstall, UNINSTALL_ERROR, UNINSTALL_FINISHED, Version};


const WINDOW_TITLE: LocalizedString<AppState> = LocalizedString::new("Rare Updater");


pub fn main() {
    let main_window = WindowDesc::new(main_widget)
        .title(WINDOW_TITLE).resizable(false)
        .window_size((400.0, 400.0));

    let config = Config::read();
    let initial_state = AppState::new(config.installed_version);

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
            data.set_error_string("".to_string());
            data.installed_version = data.latest_rare_version.clone();
            let config = Config {
                installed: true,
                installed_version: data.latest_rare_version.to_string(),
            };
            config.save();

            Handled::Yes
        } else if let Some(err) = cmd.get(ERROR) {
            data.set_error_string(err.to_string());
            data.current_screen = CurrentScreen::Error;
            Handled::Yes
        } else if let Some(gh_resp) = cmd.get(STARTUP_READY) {
            let dl_link = match gh_resp.get_windows_download_link() {
                Ok(dl_link) => dl_link,
                Err(err) => {
                    data.set_info_text(err);
                    data.current_screen = CurrentScreen::Error;
                    return Handled::Yes;
                }
            };
            let config = Config::read();
            data.latest_rare_version = Version::from_string(gh_resp.tag_name.clone());
            data.installed_version = Version::from_string(config.installed_version.clone());
            data.download_link = Some(dl_link);

            if !Version::eq(&data.latest_rare_version, &data.installed_version) && false {
                data.current_screen = CurrentScreen::Install;
                data.installing = true;
                //  install(, true, data.download_link.clone().unwrap())
            }
            if !config.installed {
                data.current_screen = CurrentScreen::Install
            } else {
                data.current_screen = CurrentScreen::Installed
            }
            Handled::Yes
        } else if let Some(err) = cmd.get(STARTUP_ERROR) {
            data.current_screen = CurrentScreen::Error;
            data.set_error_string(err.to_string());
            Handled::Yes
        } else if cmd.get(UNINSTALL_FINISHED).is_some() {
            data.installing = false;
            match Config::remove_file() {
                Ok(_) => {
                    data.current_screen = CurrentScreen::Install;
                    data.installed_version = Version::new(0, 0, 0);
                    Handled::Yes
                }
                Err(err) => {
                    data.current_screen = CurrentScreen::Error;
                    data.error_string = err.to_string();
                    Handled::Yes
                }
            }
        } else if let Some(err) = cmd.get(UNINSTALL_ERROR) {
            data.set_error_string(err.to_string());
            data.installing = false;
            Handled::Yes
        } else {
            Handled::No
        }
    }
}

fn launch_rare() {
    let exe_path = data_local_dir().unwrap().join("Rare").join("Python").join("rare.exe");
    let _ = Popen::create(&[exe_path.into_os_string().into_string().to_owned().unwrap().as_str()],
                          PopenConfig {
                              detached: true,
                              ..Default::default()
                          });
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
    let title_label = Padding::new((0.0, 20.0), Label::new("Rare Installer").with_text_size(30.0));

    let page_switch = ViewSwitcher::new(
        |data: &AppState, _env| data.current_screen.clone(),
        |selector, _data, _env| match selector {
            CurrentScreen::Install => {
                Box::new(install_screen())
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

    return Flex::column()
        .with_child(title_label)
        .with_child(page_switch);
}

fn installed_screen() -> impl Widget<AppState> {
    let version_row = Either::new(
        |data: &AppState, _| {
            !Version::eq(&data.installed_version, &data.latest_rare_version)
        },
        Flex::row()
            .with_child(Label::new(
                |data: &AppState, _: &Env| {
                    format!("Update available: {} -> {}",
                            data.installed_version.to_string(),
                            data.latest_rare_version.to_string())
                }
            ))
            .with_child(
                Button::new("Update")
                    .on_click(|ctx: &mut EventCtx, data: &mut AppState, _env: &Env| {
                        let installs = install(ctx.get_external_handle(), true, data.download_link.clone().unwrap());
                        if installs {
                            data.current_screen = CurrentScreen::Install;
                            data.installing = true;
                        } else {
                            data.set_error_string("Files are locked. Please kill rare first".to_string());
                        }
                    })
            ),
        Label::new(|data: &AppState, _env: &Env| { format!("Version {} installed", data.installed_version.to_string()) }),
    );

    let launch_button = Button::new("Launch")
        .on_click(|_: &mut EventCtx, _: &mut AppState, _env: &Env| {
            launch_rare();
        });


    let uninstall_button = Either::new(|data: &AppState, _| {
        !data.installing
    },
                                       Flex::column().with_child(Button::new("Uninstall")
                                           .on_click(
                                               |ctx: &mut EventCtx, data: &mut AppState, _env: &Env| {
                                                   if uninstall(ctx.get_external_handle(), false) {
                                                       data.installing = true;
                                                       println!("Uninstall Rare");
                                                   } else {
                                                       data.set_error_string("Files are locked. Please kill rare first".to_string());
                                                   }
                                               }
                                           )).with_child(launch_button),
                                       Label::new("Uninstalling"),
    );
    let mut info_text = Label::new(|data: &AppState, _: &Env| { data.error_string.to_string() });
    info_text.set_line_break_mode(LineBreaking::WordWrap);
    let layout = Flex::column()
        .with_child(version_row)
        .with_child(uninstall_button)
        .with_child(info_text);

    return layout;
}

fn loading_screen() -> impl Widget<AppState> {
    return Label::new("Loading...").center();
}

fn error_screen() -> impl Widget<AppState> {
    let title = Label::new("Oops, an error occurred").with_text_size(20.0);

    let mut error_text = Label::new(|data: &AppState, _: &Env| { data.error_string.to_string() });
    error_text.set_line_break_mode(LineBreaking::WordWrap);
    return Flex::column()
        .with_child(title)
        .with_child(error_text);
}

fn install_screen() -> impl Widget<AppState> {
    // let versions = [("Stable", Version::Stable), ("Git", Version::Git)];

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
            .with_child(install_button).center(),
        Label::new("Installing").center(),
    );

    let layout = Flex::column()
        // .with_child(pypresence_checkbox)
        // .with_child(webview_checkbox)
        //  .with_child(radio)
        .with_child(info_text)
        .with_child(button_layout);
    return layout;
}
