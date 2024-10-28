use serenity::async_trait;
use serenity::model::channel::Message;
use serenity::prelude::*;
use serenity::Client;
use std::ffi::{CString, c_char};
use std::sync::{Arc, Mutex};
use std::collections::VecDeque;
use serenity::utils::MessageBuilder;
use serenity::model::gateway::Ready;
use std::env;
use windows::Win32::Foundation::*;
use windows::Win32::System::SystemServices::*;
use windows::core::*;
use windows::Win32::UI::WindowsAndMessaging::MessageBoxA;

struct Handler {
    message_queue: Arc<Mutex<VecDeque<String>>>, // Store messages
}

#[async_trait]
impl EventHandler for Handler {
    async fn message(&self, context: Context, msg: Message) {
        // Check for "!hello" command
        if msg.content == "!hello" {
            let channel = match msg.channel_id.to_channel(&context).await {
                Ok(channel) => channel,
                Err(why) => {
                    println!("Error getting channel: {:?}", why);
                    return;
                },
            };

            // Create the response message
            let response = MessageBuilder::new()
                .push("Hello, ")
                .push_bold_safe(&msg.author.name)
                .push("! How can I assist you today?")
                .build();

            // Send the response
            if let Err(why) = msg.channel_id.say(&context.http, &response).await {
                println!("Error sending message: {:?}", why);
            }

            // Store the message content in the queue
            let mut queue = self.message_queue.lock().unwrap();
            queue.push_back(msg.content.clone());
        }
    }

    async fn ready(&self, _: Context, ready: Ready) {
        println!("Bot is connected as {}!", ready.user.name);
    }
}

static mut MESSAGE_QUEUE: Option<Arc<Mutex<VecDeque<String>>>> = None;

#[no_mangle]
pub extern "C" fn on_process_attach() {
    let token = env::var("DISCORD_TOKEN").expect("Expected a token in the environment");

    // Initialize the message queue safely
    let message_queue = Arc::new(Mutex::new(VecDeque::new()));
    unsafe {
        MESSAGE_QUEUE = Some(message_queue.clone());
    }

    // Start the Discord client asynchronously
    let handler = Handler {
        message_queue: message_queue.clone(),
    };
    
    tokio::spawn(async move {
        let mut client = Client::builder(&token)
            .event_handler(handler)
            .await
            .expect("Error creating client");

        if let Err(why) = client.start().await {
            println!("Client error: {:?}", why);
        }
    });
}

#[no_mangle]
pub extern "C" fn get_message() -> *const c_char {
    // Access the message queue safely
    let message_queue = unsafe {
        MESSAGE_QUEUE
            .as_ref()
            .expect("MESSAGE_QUEUE has not been initialized")
    };

    // Lock the queue and check for messages
    let mut queue = message_queue.lock().unwrap();
    if let Some(message) = queue.pop_front() {
        CString::new(message).unwrap().into_raw() // Return C-compatible string
    } else {
        std::ptr::null()
    }
}

#[no_mangle]
pub extern "C" fn free_message(msg: *const c_char) {
    unsafe {
        if !msg.is_null() {
            CString::from_raw(msg as *mut c_char); // Free the C string
        }
    }
}

#[no_mangle]
#[allow(non_snake_case)]
extern "system" fn DllMain(
    dll_module: HINSTANCE,
    call_reason: u32,
    _: *mut (),
) -> bool {
    match call_reason {
        DLL_PROCESS_ATTACH => attach(),
        DLL_PROCESS_DETACH => detach(),
        _ => (),
    }

    true
}

fn attach() {
    unsafe {
        MessageBoxA(HWND(std::ptr::null_mut()),
            s!("ZOMG!"),
            s!("hello.dll"),
            Default::default()
        );
    }
}

fn detach() {
    unsafe {
        MessageBoxA(HWND(std::ptr::null_mut()),
            s!("GOODBYE!"),
            s!("hello.dll"),
            Default::default()
        );
    }
}
#[cfg(test)]
mod tests {
    use super::*;
    use std::env;

    #[tokio::test]
    async fn test_on_process_attach() {
        // Set the DISCORD_TOKEN environment variable for testing
        env::set_var("DISCORD_TOKEN", "YOUR_TOKEN");

        // Call the function that starts the bot
        on_process_attach();

        // Optionally wait a moment for the bot to start
        tokio::time::sleep(tokio::time::Duration::from_millis(100)).await;

        // You may want to check that the bot is running, or any other conditions
        // Since the bot runs asynchronously, you might not be able to directly test it here
        // Instead, you can check if MESSAGE_QUEUE was initialized
        tokio::time::sleep(tokio::time::Duration::from_secs(20)).await;
        unsafe {
            assert!(MESSAGE_QUEUE.is_some(), "MESSAGE_QUEUE should be initialized");
        }
    }
}