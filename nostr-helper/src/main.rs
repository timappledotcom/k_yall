use anyhow::Result;
use clap::{Parser, Subcommand};
use nostr_sdk::prelude::*;
use std::time::Duration;

#[derive(Parser)]
#[command(name = "nostr-helper")]
#[command(about = "A simple Nostr client helper for K, Y'all")]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Generate a new private key
    GenerateKey,
    /// Get public key from private key
    GetPubkey {
        /// Private key in hex format
        #[arg(short, long)]
        private_key: String,
    },
    /// Post a text note
    Post {
        /// Private key in hex format
        #[arg(short, long)]
        private_key: String,
        /// Content to post
        #[arg(short, long)]
        content: String,
        /// Relay URLs (comma-separated)
        #[arg(short, long)]
        relays: String,
    },
}

#[tokio::main]
async fn main() -> Result<()> {
    println!("nostr-helper starting...");
    let cli = Cli::parse();
    println!("Command parsed: {:?}", std::env::args().collect::<Vec<_>>());

    match &cli.command {
        Commands::GenerateKey => {
            println!("Generating new key...");
            let keys = Keys::generate();
            println!("{}", hex::encode(keys.secret_key().as_secret_bytes()));
        }
        Commands::GetPubkey { private_key } => {
            println!("Getting public key...");
            let secret_key = SecretKey::from_hex(private_key)?;
            let keys = Keys::new(secret_key);
            println!("{}", keys.public_key());
        }
        Commands::Post {
            private_key,
            content,
            relays,
        } => {
            println!("Starting post operation...");
            // Set a timeout for the entire operation
            let timeout_duration = Duration::from_secs(20);
            match tokio::time::timeout(timeout_duration, post_to_nostr(private_key, content, relays)).await {
                Ok(result) => {
                    println!("Post operation completed");
                    result?
                },
                Err(_) => {
                    println!("Post operation timed out");
                    anyhow::bail!("Operation timed out after {} seconds", timeout_duration.as_secs())
                },
            }
        }
    }

    println!("nostr-helper finished successfully");
    Ok(())
}

async fn post_to_nostr(private_key_hex: &str, content: &str, relays_str: &str) -> Result<()> {
    // Parse private key - handle both hex and bech32 formats
    let secret_key = if private_key_hex.starts_with("nsec") {
        // Bech32 encoded (nsec format)
        SecretKey::from_bech32(private_key_hex)?
    } else {
        // Hex format
        SecretKey::from_hex(private_key_hex)?
    };
    
    let keys = Keys::new(secret_key);

    // Create client
    let client = Client::new(keys);

    // Parse and add relays
    let relay_urls: Vec<&str> = relays_str.split(',').collect();
    println!("Connecting to {} relays...", relay_urls.len());
    
    for relay_url in relay_urls {
        let relay_url = relay_url.trim();
        if !relay_url.is_empty() {
            println!("Adding relay: {}", relay_url);
            match client.add_relay(relay_url).await {
                Ok(_) => println!("Successfully added relay: {}", relay_url),
                Err(e) => println!("Failed to add relay {}: {}", relay_url, e),
            }
        }
    }

    // Connect to relays
    println!("Connecting to relays...");
    client.connect().await;

    // Wait for connections with timeout
    let mut attempts = 0;
    let max_attempts = 10;
    
    while attempts < max_attempts {
        tokio::time::sleep(Duration::from_millis(500)).await;
        
        let relay_pool = client.pool();
        let relays = relay_pool.relays().await;
        let connected_count = relays.iter().filter(|(_, relay)| relay.is_connected()).count();
        
        println!("Connection attempt {}: {} of {} relays connected", attempts + 1, connected_count, relays.len());
        
        if connected_count > 0 {
            break;
        }
        
        attempts += 1;
    }

    // Check final connection status
    let relay_pool = client.pool();
    let relays = relay_pool.relays().await;
    let connected_relays: Vec<_> = relays.iter().filter(|(_, relay)| relay.is_connected()).collect();
    
    println!("Final status: {} connected relays", connected_relays.len());
    for (url, _) in &connected_relays {
        println!("  ✓ Connected: {}", url);
    }
    
    if connected_relays.is_empty() {
        anyhow::bail!("Failed to connect to any relays");
    }

    // Create and publish text note
    println!("Creating text note...");
    let event_builder = EventBuilder::text_note(content);
    
    println!("Publishing event...");
    match client.send_event_builder(event_builder).await {
        Ok(event_id) => {
            println!("Posted event with ID: {:?}", event_id);
            
            // Wait a bit for the event to be sent
            tokio::time::sleep(Duration::from_secs(2)).await;
            
            println!("Post completed successfully!");
            Ok(())
        }
        Err(e) => {
            anyhow::bail!("Failed to send event: {}", e);
        }
    }
}
