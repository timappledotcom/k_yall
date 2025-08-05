# K, Y'all - Quick Start Guide

## What is K, Y'all?

K, Y'all is a social posting application designed specifically for KDE and Plasma desktop environments. It allows you to post simultaneously to multiple social media platforms from a single interface that lives in your system tray.

## Features Overview

### 🌐 Multi-Platform Support
- **Mastodon**: Full API support with custom server configurations
- **BlueSky**: AT Protocol integration with app password authentication  
- **MicroBlog**: Compatible with MicroBlog instances
- **Nostr**: Native protocol support with configurable relays

### 📱 User Experience
- **System Tray Integration**: Minimizes to tray, quick access via icon click
- **Multiple Accounts**: Configure multiple accounts per service
- **Image Support**: Upload up to 4 images per post
- **Character Counting**: Real-time character count with service limits
- **Drag & Drop**: Drag images directly into the post window

### ⚙️ Nostr Features
- **Pre-configured Relays**: Includes 5 popular relays by default:
  - wss://relay.damus.io
  - wss://nos.lol  
  - wss://relay.snort.social
  - wss://relay.current.fyi
  - wss://brb.io
- **Custom Relays**: Add up to 5 additional custom relays
- **Key Management**: Generate new keys or import existing ones

## Quick Setup

### 1. Installation
```bash
# Install dependencies
./install-deps.sh

# Build the application
./build.sh

# Install system-wide
cd build
sudo make install
```

### 2. First Run
1. Launch K, Y'all from your application menu
2. The app will appear in your system tray
3. Click the tray icon to open the main window
4. Click "Manage Accounts" to configure your social media accounts

### 3. Account Configuration

#### Mastodon Setup
1. Go to your Mastodon instance → Settings → Development
2. Create a new application with these scopes: `read write follow`
3. Copy the access token
4. In K, Y'all: Add Account → Mastodon
5. Enter your server URL (e.g., `https://mastodon.social`)
6. Paste your access token
7. Set a display name and save

#### BlueSky Setup  
1. In BlueSky, go to Settings → App Passwords
2. Generate a new app password
3. In K, Y'all: Add Account → BlueSky
4. Enter your BlueSky username (handle)
5. Enter the app password (not your main password!)
6. Save the account

#### Nostr Setup
1. In K, Y'all: Add Account → Nostr
2. Either generate a new key or import an existing private key
3. Configure your preferred relays (defaults are provided)
4. Save the account

## Using K, Y'all

### Creating a Post
1. Click the system tray icon or use "New Post" from the context menu
2. Type your message (max 500 characters by default)
3. Select which accounts to post to using the checkboxes
4. Optionally add images by clicking "Add Image" or dragging files
5. Click "Post" to publish to all selected accounts

### Managing Images
- **Add Images**: Click "Add Image" button or drag files into the window
- **Remove Images**: Select an image in the list and click "Remove Selected"
- **Supported Formats**: PNG, JPG, JPEG, GIF, WebP
- **Limit**: Up to 4 images per post

### Account Management
- **Edit Accounts**: Settings → Accounts tab → Select account → Edit
- **Enable/Disable**: Toggle accounts on/off without deleting
- **Default Posting**: Set which accounts are selected by default
- **Test Connection**: Verify account credentials work

### Nostr Configuration
- **Default Relays**: 5 popular relays are pre-configured
- **Custom Relays**: Add up to 5 additional relays in Settings → Nostr Settings
- **Relay Format**: URLs must start with `wss://` or `ws://`
- **Reset Option**: Restore default relay list anytime

## Tips & Tricks

### Keyboard Shortcuts
- **Ctrl+Enter**: Post (when post window is focused)
- **Esc**: Close post window
- **Ctrl+I**: Add image (when post window is focused)

### Best Practices
1. **Test Accounts**: Use "Test Connection" to verify account setup
2. **Character Limits**: Different services have different limits - K, Y'all warns you
3. **Image Optimization**: Compress large images before uploading for faster posting
4. **Backup Keys**: For Nostr, backup your private key securely
5. **App Passwords**: For BlueSky, always use app passwords, never your main password

### Troubleshooting
- **Can't Connect**: Check internet connection and account credentials
- **Posts Fail**: Verify account tokens haven't expired
- **Images Won't Upload**: Check file format and size
- **Nostr Issues**: Verify relay URLs and private key format

## Privacy & Security

- **Local Storage**: All credentials stored locally using Qt's secure settings
- **No Tracking**: K, Y'all doesn't collect or transmit user data
- **Encryption**: Account data is encrypted at rest
- **Network**: Only communicates with configured social media services

## Getting Help

- **Documentation**: Check the README.md for detailed information
- **Issues**: Report bugs on the project's issue tracker
- **Configuration**: Settings are stored in `~/.config/kyall/`

---

Enjoy posting with K, Y'all! 🎉
