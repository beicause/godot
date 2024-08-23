use json5;
use serde_json;
use toml;

pub fn json5_to_json(data: &str, pretty: bool) -> String {
    let value: json5::Result<serde_json::Value> = json5::from_str(data);
    if value.is_err() {
        println!("JSON5 deserialization error: {}", value.unwrap_err());
        return String::new();
    }
    if pretty {
        serde_json::to_string_pretty(&value.unwrap()).unwrap()
    } else {
        serde_json::to_string(&value.unwrap()).unwrap()
    }
}

pub fn json_to_json5(data: &str) -> String {
    let value: serde_json::Result<serde_json::Value> = serde_json::from_str(data);
    if value.is_err() {
        println!("JSON deserialization error: {}", value.unwrap_err());
        return String::new();
    }
    json5::to_string(&value.unwrap()).unwrap()
}

pub fn toml_to_json(data: &str, pretty: bool) -> String {
    let value: Result<toml::Value, toml::de::Error> = data.parse();
    if value.is_err() {
        println!("TOML deserialization error: {}", value.unwrap_err());
        return String::new();
    }
    if pretty {
        serde_json::to_string_pretty(&value.unwrap()).unwrap()
    } else {
        serde_json::to_string(&value.unwrap()).unwrap()
    }
}

pub fn json_to_toml(json: &str, pretty: bool) -> String {
    let value: serde_json::Result<serde_json::Value> = serde_json::from_str(json);
    if value.is_err() {
        println!("JSON deserialization error: {}", value.unwrap_err());
        return String::new();
    }
    if pretty {
        toml::to_string_pretty(&value.unwrap()).unwrap()
    } else {
        toml::to_string(&value.unwrap()).unwrap()
    }
}

#[cxx::bridge(namespace = "json_converter")]
pub mod ffi {
    extern "Rust" {
        fn json5_to_json(data: &str, pretty: bool) -> String;
        fn json_to_json5(data: &str) -> String;

        fn toml_to_json(toml: &str, pretty: bool) -> String;
        fn json_to_toml(json: &str, pretty: bool) -> String;
    }
}
