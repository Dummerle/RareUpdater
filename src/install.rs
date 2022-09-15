use std::fs::File;
use std::io::{Write};
use std::path::Path;
use std::string::String;
use std::{fs, io, thread};

use druid::{Data, Lens, Selector, Target};
use zip::result::ZipResult;

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
        let filename = match download_file(
            "https://github.com/Dummerle/Rare/releases/download/1.9.0-rc.3/Rare-Windows-1.9.0.11.zip".to_string(),
            // TODO: use env vars
            ".".to_string(),
        ) {
            Ok(filename) => filename,
            Err(err) => {
                event_sink
                    .submit_command(STATE_UPDATE, format!("Installation failed: {err}").to_string(), Target::Auto)
                    .expect("Can't send command");
                return;
            }
        };


        event_sink.submit_command(STATE_UPDATE, "Extracting package".to_string(), Target::Auto)
            .expect("Can't send command");

        match extract_zip_file(filename) {
            Ok(_) => {}
            Err(_) => {}
        }
    });
}

fn extract_zip_file(filename: String) -> Result<(), String> {
    let file = match File::open(&filename) {
        Ok(file) => { file }
        Err(err) => return Err(err.to_string())
    };

    let mut archive = match zip::ZipArchive::new(file) {
        Ok(archive) => archive,
        Err(err) => return Err(err.to_string())
    };

    for i in 0..archive.len() {
        let mut file = archive.by_index(i).unwrap();
        let outpath = match file.enclosed_name() {
            Some(path) => path.to_owned(),
            None => continue,
        };

        {
            let comment = file.comment();
            if !comment.is_empty() {
                println!("File {} comment: {}", i, comment);
            }
        }

        if (*file.name()).ends_with('/') {
            println!("File {} extracted to \"{}\"", i, outpath.display());
            fs::create_dir_all(&outpath).unwrap();
        } else {
            println!(
                "File {} extracted to \"{}\" ({} bytes)",
                i,
                outpath.display(),
                file.size()
            );
            if let Some(p) = outpath.parent() {
                if !p.exists() {
                    fs::create_dir_all(&p).unwrap();
                }
            }
            let mut outfile = fs::File::create(&outpath).unwrap();
            io::copy(&mut file, &mut outfile).unwrap();
        }
    }
    Ok(())
}


fn download_file(url: String, mut dest_path: String) -> Result<String, String> {
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

    return Ok(file_name);
}

