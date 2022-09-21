use std::fs::File;
use std::io::{Write};
use std::path::{PathBuf};
use std::string::String;
use std::{fs, io, thread};
use dirs::{cache_dir, data_local_dir};
// use subprocess::{Popen, PopenConfig, Redirection};

use druid::{Data, ExtEventSink, Lens, Selector, Target};

pub(crate) const STATE_UPDATE: Selector<String> = Selector::new("state_update");
pub(crate) const FINISHED: Selector<()> = Selector::new("finished");
pub(crate) const ERROR: Selector<String> = Selector::new("error");


#[derive(Clone, Copy, PartialEq, Data)]
pub(crate) enum Version {
    Stable,
    Git,
}

pub struct InstallOptions {
    version: Version,
}

#[derive(Clone, Data, Lens)]
pub struct AppState {
    version: Version,
    pub installing: bool,
    pub(crate) info_text: String,
}

impl AppState {
    pub(crate) fn set_info_text(&mut self, text: String) {
        self.info_text = text;
    }

    pub fn get_install_options(&self) -> InstallOptions {
        InstallOptions {
            version: self.version,
        }
    }

    pub fn default() -> AppState {
        return AppState {
            info_text: "".to_string(),
            version: Version::Stable,
            installing: false,
        };
    }
}

pub fn install(event_sink: ExtEventSink, _options: InstallOptions) {
    thread::spawn(move || {
        event_sink
            .submit_command(STATE_UPDATE, "Downloading package".to_string(), Target::Auto)
            .expect("Can't send command");

        let filename = match download_file(
            "https://github.com/Dummerle/Rare/releases/download/1.9.0/Rare-Windows-1.9.0.zip".to_string(),
            // TODO: use env vars
            cache_dir().unwrap(),
        ) {
            Ok(filename) => filename,
            Err(err) => {
                event_sink
                    .submit_command(ERROR, format!("Installation failed: {err}").to_string(), Target::Auto)
                    .expect("Can't send command");
                return;
            }
        };

        match extract_zip_file(&filename) {
            Ok(_) => {}
            Err(err) => {
                event_sink
                    .submit_command(ERROR, format!("Installation failed: {err}").to_string(), Target::Auto)
                    .expect("Can't send command");
                return;
            }
        }

        fs::remove_file(filename).unwrap();


        /*
                event_sink.submit_command(STATE_UPDATE, "Installing python".to_string(), Target::Auto)
                    .expect("Can't send command");

                let py_install_dir = match install_python(filename.as_str()) {
                    Ok(path) => path,
                    Err(err) => {
                        event_sink.submit_command(ERROR, format!("Failed to install Python: {}", err).to_string(), Target::Auto)
                            .expect("Can't send command");
                        return;
                    }
                };
                event_sink.submit_command(STATE_UPDATE, "Installing packages".to_string(), Target::Auto)
                    .expect("Can't send command");

                match install_packages(py_install_dir, _options.install_pypresence == true,
                                       _options.install_webview == true, "stable".to_string()) {
                    Ok(_) => {}
                    Err(err) => {
                        event_sink.submit_command(ERROR, format!("Failed to install packages: {}", err).to_string(), Target::Auto)
                            .expect("Can't send command");
                        return;
                    }
                };
        */
        // create desktop shortcut etc

        event_sink.submit_command(FINISHED, (), Target::Auto).expect("Failed to send command")
    });
}

fn extract_zip_file(filename: &PathBuf) -> Result<(), String> {
    let file = match File::open(&filename) {
        Ok(file) => { file }
        Err(err) => return Err(err.to_string())
    };
    let base_path = data_local_dir().unwrap().join("Rare\\Python");
    if !base_path.exists() {
        match fs::create_dir_all(&base_path) {
            Ok(_) => {}
            Err(err) => return Err(err.to_string())
        };
    }

    let mut archive = match zip::ZipArchive::new(file) {
        Ok(archive) => archive,
        Err(err) => return Err(err.to_string())
    };

    for i in 0..archive.len() {
        let mut file = archive.by_index(i).unwrap();

        let mut outpath = base_path.clone();

        let file_name = match file.enclosed_name() {
            Some(path) => path.to_owned(),
            None => continue,
        };
        outpath.push(file_name);

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
            let mut outfile = File::create(&outpath).unwrap();
            io::copy(&mut file, &mut outfile).unwrap();
        }
    }
    Ok(())
}


fn download_file(url: String, mut dest_path: PathBuf) -> Result<PathBuf, String> {
    let split = url.split("/");
    let file_name = split.last().unwrap().to_string();

    let response = match reqwest::blocking::get(url.as_str()) {
        Ok(response) => response,
        Err(err) => return Err(err.to_string())
    };
    if response.status() != 200 {
        return Err(format!("Failed to download {}", url).to_string());
    }


    dest_path.push(file_name);

    let mut file = match File::create(&dest_path) {
        Ok(file) => file,
        Err(err) => return Err(err.to_string())
    };

    let content = response.bytes().expect("Can't read bytes");

    match file.write_all(&content) {
        Ok(_) => {}
        Err(err) => return Err(err.to_string())
    };

    return Ok(dest_path);
}
/*
fn install_python(py_exe_file: &str) -> Result<String, String> {
    let path = match data_local_dir() {
        None => return Err("Can't find data dir".to_string()),
        Some(path) => path
    };
    let mut target_dir = PathBuf::new();

    target_dir.push(path);
    target_dir.push("Rare");

    target_dir.push("python");
    match fs::create_dir_all(&target_dir) {
        Ok(_) => {}
        Err(err) => return Err(err.to_string())
    };

    // there is probably a better option than chaining methods
    let mut target_str = "TargetDir=".to_string();
    target_str.push_str(&target_dir.to_str().unwrap());

    let command = [
        py_exe_file, "/quiet", py_exe_file, &target_str, "Shortcuts=0",
        "Include_doc=0", "Include_launcher=0", "Include_tcltk=0", "Include_test=0"
    ];

    let mut process = match Popen::create(
        &command, PopenConfig {
            stdout: Redirection::Pipe,
            stderr: Redirection::Pipe,
            ..Default::default()
        },
    ) {
        Ok(process) => process,
        Err(err) => return Err(err.to_string())
    };

    // Maybe use out to show debug console
    let (out, proc_err) = match process.communicate(None) {
        Ok(res) => res,
        Err(err) => return Err(err.to_string())
    };

    if out.is_some() {
        println!("{}", out.unwrap());
    }

    if let Some(exit_code) = process.poll() {
        if exit_code != subprocess::ExitStatus::Exited(0) {
            return Err(proc_err.unwrap());
        }
    }
    let (out, _) = match process.communicate(None) {
        Ok(res) => res,
        Err(err) => return Err(err.to_string())
    };

    if out.is_some() {
        println!("{}", out.unwrap());
    }

    Ok(target_dir.into_os_string().into_string().unwrap().to_string())
}

fn install_packages(python_install_dir: String, install_pypresence: bool, install_webview: bool, version: String) -> Result<(), String> {
    let mut python_exe = PathBuf::new();
    python_exe.push(python_install_dir);
    python_exe.push("python.exe");

    let rare_install;
    if version == "stable" {
        rare_install = "Rare"
    } else {
        rare_install = "git+https://github.com/Dummerle/Rare.git"
    }

    let python_exe_str = python_exe.into_os_string().into_string().unwrap();
    let mut command = vec![python_exe_str.as_str(), "-m", "pip", "install", "-U", rare_install];

    if install_pypresence {
        command.push("pypresence");
    }
    if install_webview {
        command.push("pywebview[cef]")
    }

    println!("{:?}", &command);

    let mut process = match Popen::create(
        &command,
        PopenConfig {
            stdout: Redirection::Pipe,
            stderr: Redirection::Pipe,
            ..Default::default()
        },
    ) {
        Ok(proc) => proc,
        Err(err) => return Err(err.to_string())
    };

    let (out, err) = match process.communicate(None) {
        Ok(res) => res,
        Err(err) => return Err(err.to_string())
    };
    if out.is_some() {
        println!("{}", out.unwrap());
    }
    if err.is_some() {
        println!("{}", err.unwrap());
    }


    Ok(())
}*/