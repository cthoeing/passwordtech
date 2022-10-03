# Password Tech
Password Tech is a powerful password generator for Windows, capable of generating large amounts of cryptographically-secure passwords, pronounceable passwords, pattern-based passwords, passphrases composed of words, and scripted passwords. Passwords can be stored in encrypted databases along with user names, URLs, notes, etc. Password Tech provides lots of options to customize passwords to the users' various needs, and individual scripting with Lua gives full control over the process of password generation. Text encryption is also supported.

Password Tech runs on Windows 7 and later.

## Features

- Full Unicode support
- Cryptographically-secure (using AES, ChaCha20, SHA-256, SHA-512)
- Password manager that handles encrypted databases
- Databases can be protected by a regular password and/or key file
- In addition to a regular master password, a *recovery password* can be specified
- Passphrases composed of words from a word list
- Pronounceable passwords
- Pattern-based passwords
- Password scripting with Lua
- Fully customizable
- Enryption of clipboard text

## Release Notes

The history of releases of main version 3 is contained in the file *changes.txt*.

## License

Password Tech is distributed under the terms and conditions of the GNU General Public License v2. For more details, see the file *license.txt* that comes with every release.

## Building

To build the software, you need:

- Embarcadero C++Builder 10.4 Update 2 or a later version ("Community" edition is sufficient)
- (optional) [Inno Setup](https://jrsoftware.org/isinfo.php) to create the setup of Password Tech (you should use the file *setup/PwTech.iss* for this purpose)

## Credits

Password Tech makes use of the following open source components:
  
- Implementations of AES, SHA1, SHA-256, SHA-512, and base64 encoding by Brainspark B.V. (PolarSSL/[mbed TLS](https://tls.mbed.org/) library)
- [miniLZO](https://www.lzop.org/) by Markus F.X.J. Oberhumer
- [diceware8k](http://www.diceware.com) list by Arnold G. Reinhold
- [Lua](https://www.lua.org/) interpreter and library by Lua.org, PUC-Rio
- Implementation of IDropSource and IDropTarget COM interface by [J. Brown](www.catch22.net)
- ChaCha implementation by D.J. Bernstein

