use std::fs;
use std::path::PathBuf;
use std::str::FromStr;
use dirs::data_local_dir;
use serde::{Deserialize, Serialize};

#[derive(Deserialize, Serialize)]
pub struct Config {
    pub installed: bool,
    pub installed_version: String,
}

impl Config {
    pub fn read() -> Config {
        let path = Config::get_config_path();
        if !PathBuf::from_str(path.as_str()).unwrap().exists(){
            return Config{
                installed: false,
                installed_version: "0.0.0".to_string()
            }
        }
        let data = fs::read_to_string(path).expect("Unable to read file");
        let res: Config = serde_json::from_str(&data).expect("Unable to parse");
        return res;
    }

    fn get_config_path()->String{
        data_local_dir().unwrap().join("Rare").join("config.json").into_os_string().into_string().unwrap()
    }

    pub fn save(&self) {
        fs::write(Config::get_config_path().as_str(), serde_json::to_string_pretty(self).unwrap()).expect("TODO: panic message");
    }

    pub fn remove_file() -> Result<(), String>{
        match fs::remove_file(Config::get_config_path()) {
            Ok(_) => Ok(()),
            Err(err) => Err(err.to_string())
        }
    }

}