use std::env;
use dotenv::dotenv;

fn main() {
    dotenv().ok();

    let discord_token = env::var("DISCORD_TOKEN").expect("DISCORD_TOKEN not found");

    println!("cargo:rustc-env=DISCORD_TOKEN={}", discord_token);
}
