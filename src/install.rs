use std::fs::File;
use std::io::{Write};
use std::path::{PathBuf};
use std::{fs, io, thread};
use dirs::{cache_dir, data_local_dir, desktop_dir, data_dir};
use serde::{Deserialize, Serialize};

use druid::{Data, ExtEventSink, Lens, Selector, Target};
#[cfg(windows)]
use mslnk::ShellLink;

pub(crate) const STATE_UPDATE: Selector<String> = Selector::new("state_update");
pub(crate) const INSTALLATION_FINISHED: Selector<()> = Selector::new("finished");
pub(crate) const ERROR: Selector<String> = Selector::new("error");
pub(crate) const UNINSTALL_ERROR: Selector<String> = Selector::new("uninstall_error");
pub(crate) const STARTUP_ERROR: Selector<String> = Selector::new("startup_error");
pub(crate) const STARTUP_READY: Selector<GitHubResponse> = Selector::new("gh_resp");
pub(crate) const UNINSTALL_FINISHED: Selector<()> = Selector::new("uninstall_finished");

const LOCKED_FILE: &str = "rare.exe";

#[derive(Serialize, Deserialize)]
struct Asset {
    name: String,
    browser_download_url: String,
}

#[derive(Serialize, Deserialize)]
pub struct GitHubResponse {
    url: String,
    html_url: String,
    pub tag_name: String,
    name: String,
    assets: Vec<Asset>,
}

impl GitHubResponse {
    pub fn to_string(&self) -> String {
        serde_json::to_string(self).unwrap()
    }

    pub fn get() -> Result<GitHubResponse, String> {
        let client = reqwest::blocking::Client::new();
        let resp = match client.get("https://api.github.com/repos/Dummerle/Rare/releases/latest")
            .header(reqwest::header::USER_AGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:105.0) Gecko/20100101 Firefox/105.0").send() {
            Ok(resp) => resp,
            Err(err) => return Err(err.to_string())
        };

        let body = resp.json::<GitHubResponse>().unwrap();

        return Ok(body);
    }

    pub fn get_windows_download_link(&self) -> Result<String, String> {
        for asset in self.assets.iter() {
            if asset.name.starts_with("Rare-Windows") && asset.name.ends_with(".zip") {
                return Ok(asset.browser_download_url.clone());
            }
        }
        Err("Can't find download link".to_string())
    }
}

#[derive(PartialEq, Eq, Clone, Data)]
pub enum CurrentScreen {
    Loading,
    Install,
    Error,
    Installed,
}

#[derive(Clone, Data, Lens, Eq, PartialEq)]
pub struct AppState {
    pub installing: bool,
    pub info_text: String,
    pub latest_rare_version: Version,
    pub installed_version: Version,
    pub download_link: Option<String>,
    pub current_screen: CurrentScreen,
    pub error_string: String,
}

#[derive(Clone, PartialEq, Eq, Data)]
pub struct Version {
    major: u8,
    minor: u8,
    bug_fix: u8,
}

impl Version {
    pub fn from_string(version_string: String) -> Version {
        let mut iter = version_string.splitn(3, '.');

        let major = iter.next().unwrap();
        let minor = iter.next().unwrap();
        let bug_fix = iter.next().unwrap();

        return Version {
            major: major.parse::<u8>().unwrap(),
            minor: minor.parse::<u8>().unwrap(),
            bug_fix: bug_fix.parse::<u8>().unwrap(),
        };
    }

    pub fn new(x1: u8, x2: u8, x3: u8) -> Version {
        Version {
            major: x1,
            minor: x2,
            bug_fix: x3,
        }
    }

    pub fn to_string(&self) -> String {
        return format!("{}.{}.{}", self.major, self.minor, self.bug_fix);
    }

    pub fn eq(v1: &Version, v2: &Version) -> bool {
        return v1.major == v2.major
            && v1.minor == v2.minor
            && v1.bug_fix == v2.bug_fix;
    }
}


impl AppState {
    pub(crate) fn set_info_text(&mut self, text: String) {
        self.info_text = text;
    }

    pub fn set_error_string(&mut self, text: String) {
        self.error_string = text;
    }

    pub fn new(installed_rare_version: String) -> AppState {
        return AppState {
            info_text: "".to_string(),
            latest_rare_version: Version::new(0, 0, 0),
            installing: false,
            download_link: None,
            current_screen: CurrentScreen::Loading,
            error_string: "".to_string(),
            installed_version: Version::from_string(installed_rare_version),
        };
    }


    pub fn get_download_link(&self) -> Option<String> {
        return self.download_link.clone();
    }
}

fn get_shortcut_dirs() -> Vec<PathBuf> {
    let mut paths: Vec<PathBuf> = Vec::new();

    match desktop_dir() {
        None => { print!("Can't find data local dir"); }
        Some(res) => {
            paths.push(res.join("Rare.lnk"));
        }
    }
    match data_dir() {
        None => { println!("Can't find data dir") }
        Some(mut res) => {
            res = res.join("Microsoft").join("Windows")
                .join("Start Menu").join("Programs").join("Rare.lnk");
            paths.push(res)
        }
    }

    return paths;
}

fn get_rare_dir() -> PathBuf {
    return data_local_dir().unwrap().join("Rare");
}


pub fn uninstall(event_sink: ExtEventSink, remove_data: bool) -> bool {
    if is_file_locked(get_rare_dir().join("Python").join(LOCKED_FILE)) {
        println!("File is locked. Do not uninstall");
        return false;
    }
    thread::spawn(move || {
        let mut path = get_rare_dir();
        if !remove_data {
            path = path.join("Python");
        }

        println!("Removing {}", &path.clone().into_os_string().into_string().unwrap().to_string());

        if path.exists() {
            match fs::remove_dir_all(path) {
                Ok(_) => { println!("Removed") }
                Err(err) => {
                    event_sink
                        .submit_command(UNINSTALL_ERROR, format!("Removing the directory failed: {}", err.to_string()), Target::Auto)
                        .expect("Can't send command");
                    return;
                }
            }
        }
        println!("Removing Shortcuts");

        for link in get_shortcut_dirs() {
            let link_copy = link.clone();
            match fs::remove_file(link_copy) {
                Ok(_) => {}
                Err(err) => {
                    println!("An error occurred while removing '{}' {}",
                             link.into_os_string().into_string().unwrap().as_str(), err.to_string())
                }
            };
        }

        event_sink.submit_command(UNINSTALL_FINISHED, (), Target::Auto)
            .expect("Failed to send command")
    }
    );
    return true;
}

fn is_file_locked(file: PathBuf) -> bool {
    if !file.exists() {
        println!("File does not exist");
        return false;
    }
    let mut new_file = file.clone();
    new_file.pop();
    new_file.push("test.tmp");

    match fs::copy(&file, &new_file) {
        Ok(_) => {}
        Err(err) => {
            format!("Can't move file: {err}");
            return true;
        }
    }

    match fs::remove_file(&file) {
        Ok(_) => {
            let _ = fs::rename(&new_file, &file).ok();
        }
        Err(err) => {
            format!("File is locked: {err}");
            return true;
        }
    }
    println!("File is not locked");
    return false;
}

pub fn install(event_sink: ExtEventSink, update: bool, dl_url: String) -> bool {
    let exe_path = get_rare_dir().join("Python").join(LOCKED_FILE);
    if is_file_locked(exe_path) {
        println!("File is locked. Do not update");
        return false;
    }

    thread::spawn(move || {
        if update {
            let file = get_rare_dir().join("Python");
            if file.exists() {
                match fs::remove_dir_all(file) {
                    Ok(_) => {}
                    Err(err) => {
                        event_sink
                            .submit_command(ERROR, format!("Can't remove old files: {err}").to_string(), Target::Auto)
                            .expect("Can't send command");
                        return;
                    }
                }
            }
            else{
                println!("Files do not exist. Ignore...");
            }
        }

        event_sink
            .submit_command(STATE_UPDATE, "Downloading package".to_string(), Target::Auto)
            .expect("Can't send command");

        let filename = match download_file(
            dl_url, cache_dir().unwrap(),
        ) {
            Ok(filename) => filename,
            Err(err) => {
                event_sink
                    .submit_command(ERROR, format!("Installation failed: {err}").to_string(), Target::Auto)
                    .expect("Can't send command");
                return;
            }
        };
        let base_path = data_local_dir().unwrap().join("Rare");
        event_sink
            .submit_command(STATE_UPDATE, "Extracting package".to_string(), Target::Auto)
            .expect("Can't send command");

        match extract_zip_file(&filename, base_path.clone().join("Python")) {
            Ok(_) => {}
            Err(err) => {
                event_sink
                    .submit_command(ERROR, format!("Installation failed: {err}").to_string(), Target::Auto)
                    .expect("Can't send command");
                return;
            }
        }

        event_sink
            .submit_command(STATE_UPDATE, "Cleaning up".to_string(), Target::Auto)
            .expect("Can't send command");
        fs::remove_file(filename).unwrap();

        if !update {
            create_links();
        }
        //if !update {
        //    fs::copy(std::env::current_exe().unwrap(), base_path).expect("Can't copy updater file");
        //}


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

        event_sink.submit_command(INSTALLATION_FINISHED, (), Target::Auto).expect("Failed to send command")
    });
    true
}

#[cfg(not(windows))]
fn create_links() {
    return;
}

#[cfg(windows)]
fn create_links() {
    for link in get_shortcut_dirs() {
        println!("{}", link.clone().into_os_string().into_string().unwrap().as_str());
        let target_path = data_local_dir().unwrap().join("Rare").join("Python").join("rare.exe");
        let lnk = link.into_os_string().into_string().unwrap();
        let target = target_path.into_os_string().into_string().unwrap();
        let sl = ShellLink::new(target.as_str()).unwrap();
        sl.create_lnk(lnk.as_str()).unwrap();
    }
}

fn extract_zip_file(filename: &PathBuf, base_path: PathBuf) -> Result<(), String> {
    let file = match File::open(&filename) {
        Ok(file) => { file }
        Err(err) => return Err(err.to_string())
    };
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

    for s in 0..archive.len() {
        let mut file = archive.by_index(s).unwrap();

        let mut outpath = base_path.clone();

        let file_name = match file.enclosed_name() {
            Some(path) => path.to_owned(),
            None => continue,
        };
        outpath.push(file_name);

        {
            let comment = file.comment();
            if !comment.is_empty() {
                println!("File {} comment: {}", s, comment);
            }
        }

        if (*file.name()).ends_with('/') {
            println!("File {} extracted to \"{}\"", s, outpath.display());
            fs::create_dir_all(&outpath).unwrap();
        } else {
            println!(
                "File {} extracted to \"{}\" ({} bytes)",
                s,
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