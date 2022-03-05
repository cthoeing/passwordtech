// CryptClip.h
//
// PASSWORD TECH
// Copyright (c) 2002-2022 by Christian Thoeing <c.thoeing@web.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//---------------------------------------------------------------------------
#ifndef CryptTextH
#define CryptTextH
//---------------------------------------------------------------------------
#include "RandomGenerator.h"
#include "SecureMem.h"

const int
CRYPTTEXT_VERSION = 3;

const int
CRYPTTEXT_OK                  = 0,
CRYPTTEXT_ERROR_CLIPBOARD     = 1,
CRYPTTEXT_ERROR_NOTEXT        = 2,
CRYPTTEXT_ERROR_TEXTTOOLONG   = 3,
CRYPTTEXT_ERROR_OUTOFMEMORY   = 4,
CRYPTTEXT_ERROR_TEXTCORRUPTED = 5,
CRYPTTEXT_ERROR_BADKEY        = 6,
CRYPTTEXT_ERROR_DECOMPRFAILED = 7;

const word32
CRYPTTEXT_MAXTEXTBYTES = 134217728; // 128MB

// Compresses and encrypts the clipboard text using AES in CBC mode.
// The user key (password) and a random IV are hashed 8192 times to create
// the "crunched" 256-bit key which is used to initialize the AES cipher and
// the SHA-256 message authentication code (HMAC).
// The clipboard text is then compressed using LZO (--> minilzo.c) and
// encrypted with the AES cipher afterwards. The HMAC is computed from the
// compressed plaintext. The ciphertext is copied to the clipboard as a
// base64-encoded string with line breaks after 76 characters.
// Encrypted texts have the following structure:
//
//    [IV]    [header]     [ciphertext]      [HMAC]
//  16 bytes  8 bytes     <variable size>   32 bytes
//           |        encrypted data      |
//
//
// The header contains a "magic" 3-byte identification string, the version
// of PWGen which the was used for encrypting the data, and the length
// of the uncompressed text (this information is required for decompression).
// The header and the compressed text are encrypted, whereas the IV and the HMAC
// are stored as plaintext. The length of the last encrypted block is stored in
// the first 4 bits of the 16th byte of the IV.
// Due to the structure above, encrypted texts have a minimum length of
// 90 bytes.

// encrypts the clipboard text
// -> pointer to text buffer (null-terminated string)
//    if NULL, clipboard contents will be processed
// -> password
// -> password length
// -> random generator to create the IV
// <- error code
int EncryptText(const SecureWString* psText,
  const word8* pPassw,
  int nPasswLen,
  RandomGenerator* pRandGen);


// decrypts the clipboard text
// -> pointer to text buffer (null-terminated string)
//    if NULL, clipboard contents will be processed
// -> password
// -> password length
// -> module version with which the text was encrypted
// <- error code
int DecryptText(const SecureWString* psText,
  const word8* pPassw,
  int nPasswLen,
  int nVersion = CRYPTTEXT_VERSION);

#endif
