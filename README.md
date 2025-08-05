# K, Y'all 🗣️

A modern, cross-platform social media posting application built with Qt6 and KDE Frameworks. Post to multiple social networks simultaneously with a clean, native desktop interface.

![K, Y'all Logo](kyall.svg)

## Features

#1. **Clone and build**:
```bash
git clone https://github.com/timappledotcom/k_yall.git
cd k_yall
cd nostr-helper && cargo build --release && cd ..
mkdir build && cd build && cmake .. && make
```*Multi-Platform Posting**
- **Mastodon** - Full support with image uploads
- **BlueSky** - Complete AT Protocol integration with media support  
- **Nostr** - Decentralized social networking with Rust-powered reliability
- **Simultaneous posting** to all configured accounts

### 🖼️ **Rich Media Support**
- Image uploads to supported platforms
- Drag & drop image support
- Multiple image formats (PNG, JPEG, GIF, WebP)
- Automatic MIME type detection

### 🎨 **Modern Desktop Integration**
- **System tray integration** with KDE StatusNotifierItem
- **Custom speech bubble icon** for professional branding
- **Native Qt6/KDE interface** that follows system themes
- **Minimized startup** - runs quietly in system tray

### 🔐 **Secure Authentication**
- OAuth support for Mastodon
- App passwords for BlueSky
- Private key management for Nostr
- Secure credential storage

### ⚡ **Hybrid Architecture**
- **C++ Qt6 frontend** for native desktop performance
- **Rust backend** for Nostr using industry-standard `nostr-sdk`
- **Async operations** - non-blocking UI during posts
- **Robust error handling** with detailed feedback

## Screenshots

### Main Interface
The clean, minimal interface focuses on your content:
- Character counter with platform limits
- Account selection checkboxes
- Image attachment area
- Real-time posting status

### System Tray
Runs quietly in your system tray with the custom speech bubble icon, ready when you need to share something.

## Installation

### Prerequisites

**System Dependencies:**
```bash
# Fedora/RHEL/CentOS
sudo dnf install qt6-qtbase-devel qt6-qtwebsockets-devel kf6-ki18n-devel \
    kf6-kcoreaddons-devel kf6-kconfig-devel kf6-kconfigwidgets-devel \
    kf6-kstatusnotifieritem-devel kf6-knotifications-devel kf6-kio-devel \
    libsecp256k1-devel cmake gcc-c++ extra-cmake-modules

# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-websockets-dev libkf6i18n-dev \
    libkf6coreaddons-dev libkf6config-dev libkf6configwidgets-dev \
    libkf6statusnotifieritem-dev libkf6notifications-dev libkf6kio-dev \
    libsecp256k1-dev cmake build-essential extra-cmake-modules

# Arch Linux
sudo pacman -S qt6-base qt6-websockets kf6-ki18n kf6-kcoreaddons \
    kf6-kconfig kf6-kconfigwidgets kf6-kstatusnotifieritem \
    kf6-knotifications kf6-kio libsecp256k1 cmake extra-cmake-modules
```

**Rust Toolchain:**
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
source ~/.cargo/env
```

### Building from Source

1. **Clone the repository:**
```bash
git clone https://github.com/timappledotcom/k_yall.git
cd k_yall
```

2. **Build the Rust Nostr helper:**
```bash
cd nostr-helper
cargo build --release
cd ..
```

3. **Build the Qt application:**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

4. **Install system-wide:**
```bash
sudo make install
sudo cp nostr-helper/target/release/nostr-helper /usr/bin/
sudo chmod +x /usr/bin/nostr-helper
sudo update-desktop-database
```

### Binary Releases

Download pre-built binaries from the [Releases](https://github.com/timappledotcom/k_yall/releases) page.

## Configuration

### Setting Up Accounts

1. **Launch K, Y'all** from your application menu or run `kyall`
2. **Click "Manage Accounts"** to open the settings dialog
3. **Add accounts** for each platform:

#### Mastodon Setup
- **Server**: Your instance URL (e.g., `mastodon.social`)
- **Username**: Your handle (e.g., `@username@mastodon.social`)
- **Access Token**: Generate from Settings → Development → New Application

#### BlueSky Setup  
- **Server**: `bsky.social` (or your PDS URL)
- **Username**: Your handle (e.g., `username.bsky.social`)
- **App Password**: Generate from Settings → App Passwords

#### Nostr Setup
- **Private Key**: Your nsec key or hex private key
- **Relays**: Configure up to 10 relays (5 defaults provided)

### Relay Configuration

The app comes with 5 default Nostr relays:
- `wss://relay.damus.io`
- `wss://nos.lol` 
- `wss://relay.snort.social`
- `wss://relay.current.fyi`
- `wss://brb.io`

You can add up to 5 additional custom relays in the Nostr Settings tab.

## Usage

### Basic Posting

1. **Open the post dialog** by clicking the system tray icon or running `kyall`
2. **Write your message** in the text area
3. **Select target accounts** using the checkboxes
4. **Attach images** (optional) using the "Add Image" button or drag & drop
5. **Click "Post"** to publish to all selected platforms

### Advanced Features

#### Character Limits
- Automatic character counting per platform
- Visual feedback when approaching limits
- Smart truncation suggestions

#### Image Handling
- **Mastodon**: Up to 4 images, various formats
- **BlueSky**: Multiple images with proper blob references
- **Nostr**: Image support via NIP-94 (planned)

#### Error Handling
- Detailed error messages for failed posts
- Retry capabilities for network issues
- Graceful degradation when services are unavailable

## Architecture

### Hybrid Design Philosophy

K, Y'all uses a unique hybrid architecture that combines the best of both worlds:

**Qt6/C++ Frontend:**
- Native desktop performance and integration
- KDE Frameworks for system tray, notifications, and theming
- Mature networking stack with QNetworkAccessManager
- Rich widget set for complex UI interactions

**Rust Backend (Nostr):**
- Industry-standard `nostr-sdk` crate for robust protocol support
- Memory safety and performance for cryptographic operations
- Async/await for non-blocking relay communications
- Comprehensive NIP (Nostr Implementation Possibilities) support

### Component Overview

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Qt6 Frontend  │    │   Service Layer  │    │  Rust Helper   │
│                 │    │                  │    │                 │
│ • MainWindow    │◄──►│ • AccountManager │    │ • nostr-sdk     │
│ • PostWidget    │    │ • ServiceImpl    │◄──►│ • secp256k1     │
│ • SystemTray    │    │ • NostrService   │    │ • tokio async   │
│ • Settings      │    │ • MastodonSvc    │    │ • relay mgmt    │
└─────────────────┘    │ • BlueskySvc     │    └─────────────────┘
                       └──────────────────┘
```

### File Structure

```
k_yall/
├── src/                      # Qt6/C++ source files
│   ├── main.cpp             # Application entry point
│   ├── mainwindow.*         # Main window and system tray
│   ├── postwidget.*         # Post composition interface  
│   ├── accountmanager.*     # Account management logic
│   ├── *service.*           # Platform-specific implementations
│   └── settings*.*          # Configuration dialogs
├── nostr-helper/            # Rust Nostr implementation
│   ├── src/main.rs         # CLI interface for Nostr operations
│   └── Cargo.toml          # Rust dependencies
├── resources/               # Qt resources (icons, UI files)
│   ├── icons/              # Platform and app icons
│   └── resources.qrc       # Qt resource collection
├── CMakeLists.txt          # Build configuration
├── kyall.desktop           # Desktop integration file
└── kyall.svg              # Application icon
```

### Communication Flow

1. **User Interaction**: Qt6 frontend captures user input and settings
2. **Service Dispatch**: C++ service layer manages account credentials and API calls
3. **Nostr Operations**: QProcess spawns Rust helper for cryptographic operations
4. **Network Operations**: Qt's QNetworkAccessManager handles HTTP/WebSocket for other platforms
5. **Status Updates**: Real-time feedback through Qt signals/slots system

## Development

### Development Setup

1. **Install development dependencies** (see Installation section)
2. **Clone and build**:
```bash
git clone https://github.com/yourusername/k_yall.git
cd k_yall
cd nostr-helper && cargo build --release && cd ..
mkdir build && cd build && cmake .. && make
```

3. **Run from build directory**:
```bash
./kyall
```

### Code Style

- **C++**: Follow Qt coding conventions with camelCase
- **Rust**: Standard rustfmt formatting with `cargo fmt`
- **CMake**: Proper target management and dependency handling

### Testing

**Manual Testing:**
```bash
# Test Rust helper independently
cd nostr-helper
cargo run -- generate-key
cargo run -- post --content "Test post" --private-key <key>

# Test C++ components
cd build && make test  # (when unit tests are implemented)
```

### Adding New Platforms

To add support for a new social platform:

1. **Create service class** in `src/newplatformservice.h/cpp`
2. **Implement ServiceInterface** methods:
   - `configure()`
   - `post()`
   - `uploadImage()` (if supported)
3. **Add UI elements** in `src/accountmanager.cpp`
4. **Register in main** application in `src/mainwindow.cpp`

### Contributing

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Make your changes** following the code style guidelines
4. **Test thoroughly** on both posting and account management
5. **Commit with clear messages**: `git commit -m 'Add amazing feature'`
6. **Push to your fork**: `git push origin feature/amazing-feature`
7. **Open a Pull Request** with a clear description of changes

## Troubleshooting

### Common Issues

**Application won't start:**
```bash
# Check if all dependencies are installed
ldd /usr/bin/kyall

# Verify Rust helper is executable
which nostr-helper
nostr-helper --help
```

**Posting fails:**
- **Check account credentials** in Account Manager
- **Verify network connectivity** to platform APIs
- **Check application logs** for detailed error messages

**System tray icon missing:**
- Ensure you're running in a compatible desktop environment (KDE Plasma, GNOME with tray extension)
- Check system tray settings in your desktop environment

**Nostr posts failing:**
```bash
# Test Rust helper directly
nostr-helper post --content "Test" --private-key your_key_here

# Check relay connectivity
ping relay.damus.io
```

### Debug Mode

Enable verbose logging:
```bash
export QT_LOGGING_RULES="*=true"
kyall
```

### Log Locations

- **Application logs**: Console output when run from terminal
- **Rust helper logs**: Embedded in C++ application output
- **Network logs**: Qt6 network debugging when enabled

## Platform-Specific Notes

### Mastodon
- Requires OAuth application registration on your instance
- Supports up to 4 images per post
- Character limit varies by instance (usually 500)

### BlueSky
- Uses AT Protocol with app passwords
- Requires blob upload for images before posting
- 300 character limit for posts

### Nostr
- Decentralized protocol using relay networks
- Posts are cryptographically signed
- No character limits (though clients may impose them)
- Image support through NIP-94 (future implementation)

## License

This project is licensed under the GPL v3.0 License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Qt Project** for the excellent cross-platform framework
- **KDE Community** for the desktop integration frameworks
- **Rust Nostr Community** for the robust `nostr-sdk` crate
- **Social Media Platforms** for providing open APIs
- **Contributors** who help improve this project

## Support

- **Issues**: Report bugs on [GitHub Issues](https://github.com/timappledotcom/k_yall/issues)
- **Discussions**: Join conversations in [GitHub Discussions](https://github.com/timappledotcom/k_yall/discussions)
- **Matrix**: Chat with developers in `#kyall:matrix.org` (coming soon)

---

**Made with ❤️ for the open social web**
- **Nostr Integration**: Pre-configured with 5 popular Nostr relays, ability to add 5 more custom relays
- **KDE Integration**: Native KDE/Qt application with proper theming and notifications

## Supported Services

### Mastodon
- Full Mastodon API support
- OAuth authentication (planned)
- Image uploads via media API
- Custom server support

### BlueSky
- AT Protocol integration
- App password authentication
- Image uploads via blob storage
- Automatic session management

### MicroBlog
- Compatible with MicroBlog instances
- Similar to Mastodon API
- Custom server support

### Nostr
- Native Nostr protocol support
- Pre-configured with popular relays:
  - wss://relay.damus.io
  - wss://nos.lol
  - wss://relay.snort.social
  - wss://relay.current.fyi
  - wss://brb.io
- Support for up to 10 total relays (5 default + 5 custom)
- Image uploads via nostr.build

## Building

### Dependencies

- Qt6 (Core, Widgets, Network, Gui, WebSockets)
- KDE Frameworks 6:
  - KI18n
  - KCoreAddons
  - KConfig
  - KConfigWidgets
  - KStatusNotifierItem
  - KNotifications
  - KIO
- CMake 3.16+
- C++17 compiler

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Development Dependencies (Ubuntu/Debian)

```bash
sudo apt install cmake build-essential qtbase6-dev qt6-websockets-dev \
    libkf6i18n-dev libkf6coreaddons-dev libkf6config-dev \
    libkf6configwidgets-dev libkf6statusnotifieritem-dev \
    libkf6notifications-dev libkf6kio-dev
```

## Usage

1. **Starting**: Launch K, Y'all from your application menu or run `kyall`
2. **System Tray**: The app will minimize to your system tray
3. **New Post**: Click the tray icon or use the context menu to open the post window
4. **Account Setup**: Use the Settings dialog to configure your social media accounts
5. **Posting**: Select which accounts to post to, add text and images, then post

### Account Configuration

#### Mastodon/MicroBlog
1. Go to your instance's Settings > Development
2. Create a new application
3. Copy the access token
4. Enter your server URL and access token in K, Y'all

#### BlueSky
1. Generate an app password in BlueSky settings
2. Enter your username and app password in K, Y'all

#### Nostr
1. Generate a new private key or import an existing one
2. Configure your preferred relays (defaults are provided)
3. Your public key will be derived automatically

## Configuration

Settings are stored in `~/.config/kyall/` and include:
- Account credentials (encrypted)
- Relay configurations
- UI preferences
- Default posting accounts

## Security

- Account credentials are stored securely using Qt's settings encryption
- Private keys for Nostr are stored encrypted locally
- No credentials are transmitted except to their respective services

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## License

GPL v3 - See LICENSE file for details

## Roadmap

- [ ] OAuth authentication for Mastodon
- [ ] Scheduled posting
- [ ] Draft posts
- [ ] Character count per service
- [ ] Post history/analytics
- [ ] Hashtag suggestions
- [ ] Mentions autocomplete
- [ ] Theme customization
- [ ] Keyboard shortcuts
- [ ] Plugin system for additional services
