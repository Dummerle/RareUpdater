use std::fs::File;
use std::io::{Write};
use std::path::Path;
use std::string::String;
use std::{thread};

use druid::{Data, Lens, Selector, Target};

pub(crate) const STATE_UPDATE: Selector<String> = Selector::new("state_update");

#[derive(Clone, Copy, PartialEq, Data)]
pub(crate) enum Version {
    Stable,
    Git,
}

#[derive(Clone, Data, Lens)]
pub struct AppState {
    install_pypresence: bool,
    install_webview: bool,
    version: Version,
    pub(crate) info_text: String,
}

impl AppState {
    pub(crate) fn set_info_text(&mut self, text: String) {
        self.info_text = text;
    }

    pub fn default() -> AppState {
        return AppState {
            install_pypresence: false,
            install_webview: false,
            version: Version::Stable,
            info_text: "".to_string(),
        };
    }
}

pub fn install(event_sink: druid::ExtEventSink, options: &mut AppState) {
    thread::spawn(move || {
        event_sink
            .submit_command(STATE_UPDATE, "Downloading Rare package".to_string(), Target::Auto)
            .expect("Can't send command");
        let res = download_file(
            "https://github.com/Dummerle/Rare/releases/download/1.9.0-rc.3/Rare-Windows-1.9.0.11.zip".to_string(),
            // TODO: use env vars
            ".".to_string(),
        );

        if res.is_err() {
            event_sink
                .submit_command(STATE_UPDATE, format!("Installation failed: {}", 5).to_string(), Target::Auto)
                .expect("Can't send command");
            return;
        }

        event_sink.submit_command(STATE_UPDATE, "Extracting package".to_string(), Target::Auto)
            .expect("Can't send command");
    });
}


fn download_file(url: String, mut dest_path: String) -> Result<(), String> {
    let split = url.split("/");
    let file_name = split.last().unwrap().to_string();

    let response = match reqwest::blocking::get(url.as_str()) {
        Ok(response) => response,
        Err(err) => return Err(err.to_string())
    };
    if response.status() != 200 {
        return Err(format!("Failed to download {}", url).to_string());
    }

    dest_path.push_str("/");
    dest_path.push_str(&*file_name);
    let path = Path::new(dest_path.as_str());

    let mut file = match File::create(&path) {
        Ok(file) => file,
        Err(err) => return Err(err.to_string())
    };

    let content = response.bytes().expect("Can't read bytes");

    match file.write_all(&content) {
        Ok(_) => {}
        Err(err) => return Err(err.to_string())
    };

    return Ok(());
}

