# Albert launcher

[![GitHub tag (latest by date)](https://img.shields.io/github/v/tag/albertlauncher/albert)](https://github.com/albertlauncher/albert/tags)
[![Docker CI Status](https://github.com/albertlauncher/albert/actions/workflows/ci.yml/badge.svg?event=push)](https://github.com/albertlauncher/albert/actions/workflows/ci.yml) 
[![OBS status](https://build.opensuse.org/projects/home:manuelschneid3r/packages/albert-master/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:manuelschneid3r/albert)
[![Telegram community chat](https://img.shields.io/badge/chat-telegram-0088cc.svg?style=flat)](https://telegram.me/albert_launcher_community)
[![Discord](https://img.shields.io/badge/chat-discord-7289da.svg?style=flat)](https://discord.gg/t8G2EkvRZh)
[![IRC](https://img.shields.io/badge/chat-IRC-brightgreen.svg)](irc://irc.libera.chat/albertlauncher)
[![IRC Web Client](https://img.shields.io/badge/chat-IRC_Web_Client-brightgreen.svg)](https://web.libera.chat/#albertlauncher)

Albert is a plugin based, desktop agnostic C++/Qt keyboard launcher that helps you to accomplish your workflows in a breeze.

## Contents
- [Features](#features)
- [Getting Started](#getting-started)
  - [Installation](#installation)
  - [Usage](#usage)
  - [Extension](#extension)
- [Contributions](#contributions)
- [Support and Feedback](#support-and-feedback)

## Features
- **Usability**: Albert is your shortcut to a more efficient workflow. Launch applications, search the web, or execute custom commands with ease.
- **Performance**: Lightning-fast responsiveness ensures you stay productive without delay.
- **Extensibility**: Customize and extend Albert's functionality through a wide range of plugins.
- **Beauty**: An elegant and intuitive design that enhances your desktop experience.

## Getting Started
### Installation
Unofficial packages and repository sources may contain malicious code!
Please make sure to use the sources mentioned here to install albert.
#### Using official Albert repositories
1. Visit [OBS Software Repo](https://software.opensuse.org/download.html?project=home:manuelschneid3r&package=albert).
2. Choose your Linux distribution from the available options: Arch, Debian, Fedora, openSUSE, Raspbian, Ubuntu.
3. Add repository and install manually using provided instructions.

   
If the list is lacking one of the latest distributions join the chats and let us know.
There is a also bleeding edge [master branch package](https://software.opensuse.org//download.html?project=home%3Amanuelschneid3r&package=albert-master) available.
#### Building from source
Building from sources is the least convenient, but most flexible way. This way is usually for developers only. The build process is trivial, but you have to manage the dependencies on your own. See the [OBS package specs](https://build.opensuse.org/package/show/home:manuelschneid3r/albert) for up to date build and runtime dependencies.
1. Install [CMake](https://cmake.org/download/).
2. Clone, build, and install Albert.

```bash
git clone --recursive https://github.com/albertlauncher/albert.git
cmake -B build -S albert -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cmake --install build
```

### Usage
As you would expect from a launcher, the main use case is to type a query into an input box and finally to interact with some sort of results. Actually there is not much more Albert allows you to do - triggering things. No more, no less.

For reference, visit the [*website*](https://albertlauncher.github.io/) to see all keys you can use to control Albert (v0.18).

### Extension
Albert has a flexible, nested plugin system, which gives users and developers the ability to extend its functionality.
#### Native plugins
The native way is to use C++/Qt to write a QPlugin. This gives you the performance of C++ and a host of other advantages. See the docs on [native plugins](https://albertlauncher.github.io/reference/) especially the [albert namespace](https://albertlauncher.github.io/reference/namespacealbert.html). Also check the [plugins](https://github.com/albertlauncher/plugins) repo for an up to date list of plugins.
#### Python plugins
The Python plugin adds plugins via Python modules. Check the docs of the [Python plugin](https://github.com/albertlauncher/plugins/blob/master/python/README.md) and the [python](https://github.com/albertlauncher/python) repo for an up to date list of plugins.
## Contributions
We welcome contributions from the community! If you would like to get involved, feel free to:
- Spread the word and leave a star.
- Join the community and help other users.
- Create and/or maintain official plugins.
- Use the application and report issues.
- Check the [issues](https://github.com/albertlauncher/albert/issues) and try to help debugging.
- Improve the documentation.
- Support me as a creator by [becoming a sponsor or donating](https://albertlauncher.github.io/donation/) some bucks.
## License
Distributed under the Manuel Schneider Albert license v1.0. See [LICENSE.md](https://github.com/albertlauncher/albert/blob/master/LICENSE.md) for more information.

## Support and Feedback

Your feedback is essential in making Albert better. If you encounter any issues or have suggestions for improvements, please let us know through our community channels: 
- [Telegram Community Chat](https://t.me/albertlauncher)
- [Discord](https://discord.com/invite/5Eg5yjJ)
- [IRC](https://webchat.freenode.net/?channels=#albert)


You can also Visit the [website](https://albertlauncher.github.io/) for more documentation on [*installing*](https://albertlauncher.github.io/installing/), [*using*](https://albertlauncher.github.io/using/), [*extending*](https://albertlauncher.github.io/extending/) or [*troubleshooting*](https://albertlauncher.github.io/help/) Albert.

Have fun with Albert!!! If you do not, [tell me why](https://telegram.me/albert_launcher_community). If you like it, feel free to [tip me](https://albertlauncher.github.io/donation/).
