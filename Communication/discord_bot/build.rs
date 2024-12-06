// build.rs
use std::env;
use dotenv::dotenv;

fn main() {
    dotenv().ok();
    if let Ok(token) = env::var("DISCORD_TOKEN") {
        println!("cargo:rerun-if-env-changed=DISCORD_TOKEN");
        println!("cargo:rustc-env=DISCORD_TOKEN={}", token);
    } else {
        panic!("DISCORD_TOKEN environment variable not set!");
    }
}