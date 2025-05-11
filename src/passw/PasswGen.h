// PasswGen.h
//
// PASSWORD TECH
// Copyright (c) 2002-2025 by Christian Thoeing <c.thoeing@web.de>
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
#ifndef PasswGenH
#define PasswGenH
//---------------------------------------------------------------------------
#include <vector>
#include <memory>
#include <optional>
#include "SecureMem.h"
#include "RandomGenerator.h"
#include "UnicodeUtil.h"


const int
WORDLIST_DEFAULT_SIZE           = 8192,
WORDLIST_MAX_WORDLEN            = 100,
WORDLIST_MAX_SIZE               = 1048576,

PASSWGEN_NUMCHARSETCODES        = 11,
PASSWGEN_NUMINCLUDECHARSETS     = 4,
PASSWGEN_NUMFORMATCHARSETS      = 22,

PASSW_FLAG_FIRSTCHARNOTLC       = 0x0001,  // first char must not be lower-case letter
PASSW_FLAG_EXCLUDEREPCHARS      = 0x0002,  // exclusion of repeating consecutive chars
PASSW_FLAG_INCLUDESUBSET        = 0x0004,  // include only additional characters
                                           // if custom set contains appropriate subset
PASSW_FLAG_INCLUDEUPPERCASE     = 0x0008,  // include one char from additional charset
PASSW_FLAG_INCLUDELOWERCASE     = 0x0010,
PASSW_FLAG_INCLUDEDIGIT         = 0x0020,
PASSW_FLAG_INCLUDESPECIAL       = 0x0040,
PASSW_FLAG_PHONETICUPPERCASE    = 0x0080,
PASSW_FLAG_PHONETICMIXEDCASE    = 0x0100, // mixed-case characters in phonetic passwords
PASSW_FLAG_EACHCHARONLYONCE     = 0x0200, // each character must occur only once
PASSW_FLAG_CHECKDUPLICATESBYSET = 0x0400,
PASSW_FLAG_REMOVEWHITESPACE     = 0x0800,

PASSPHR_FLAG_COMBINEWCH         = 0x0001,  // combine words & chars
PASSPHR_FLAG_CAPITALIZEWORDS    = 0x0002,  // capitalize first letter of word
PASSPHR_FLAG_DONTSEPWORDS       = 0x0004,  // don't separate words by a space
PASSPHR_FLAG_DONTSEPWCH         = 0x0008,  // don't separate words & chars by '-'
PASSPHR_FLAG_REVERSEWCHORDER    = 0x0010,
PASSPHR_FLAG_EACHWORDONLYONCE   = 0x0020,

PASSFORMAT_FLAG_EXCLUDEREPCHARS = 0x0001,
PASSFORMAT_FLAG_REMOVEWHITESPACE= 0x0002,

PASSFORMAT_PWUSED_NOSPECIFIER   = -1, // password provided, but no "P" specifier
PASSFORMAT_PWUSED_EMPTYPASSW    = -2; // "%P" specified, but no password available

const char CHARSET_SYMBOLS[] =
  "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

enum CharSetType {
  cstStandard,
  cstStandardWithFreq,
  cstPhonetic,
  cstPhoneticUpperCase,
  cstPhoneticMixedCase
};


class PasswordGenerator
{
private:
  RandomGenerator* m_pRandGen;
  w32string m_sCustomCharSet;
  using CharSetFreq = std::vector<std::pair<w32string,int>>;
  std::optional<CharSetFreq> m_customCharSetFreq;
  CharSetType m_customCharSetType;
  int m_nCustomCharSetUniqueSize;
  //int m_nCustomCharSetSize;
  double m_dCustomCharSetEntropy;
  bool m_blCustomCharSetNonLC;
  w32string m_charSetDecodes[PASSWGEN_NUMCHARSETCODES];
  w32string m_includeCharSets[PASSWGEN_NUMINCLUDECHARSETS];
  w32string m_customSubsets[PASSWGEN_NUMINCLUDECHARSETS];
  w32string m_formatCharSets[PASSWGEN_NUMFORMATCHARSETS];
  w32string m_sWordSep;
  w32string m_sWordCharSep;
  std::vector<std::wstring> m_wordList;
  int m_nWordListSize;
  double m_dWordListEntropy;
  w32string m_sAmbigCharSet;
  std::vector<w32string> m_ambigGroups;
  word32 m_lPhoneticSigma;
  std::vector<word32> m_phoneticTris;
  double m_dPhoneticEntropy;

  // convert ("parse") the input string into a "unique" character set
  // -> input string
  // -> receives CharSetFreq data if valid (may be nullptr)
  // <- optional pair of
  //    - string containing character set;
  //    - type of character set
  std::optional<std::pair<w32string,CharSetType>> ParseCharSet(
    w32string sInput,
    bool blIncludeCharFromEachSubset = false,
    std::optional<CharSetFreq>* pCharSetFreq = nullptr) const;

  WString GetCustomCharSetAsWString(void) const
  {
    return W32StringToWString(m_sCustomCharSet);
  }

public:
  // constructor
  // -> pointer to a random generator
  PasswordGenerator(RandomGenerator* pRandGen);

  // destructor
  ~PasswordGenerator();

  // makes a character set unique, i.e., converts the source string into
  // a character set where each character must occur only once
  // -> source string
  // -> string with ambiguous characters to be excluded from the final set
  // -> string receiving ambiguous characters that have been removed from
  //    the source string
  // <- unique character set
  static w32string MakeCharSetUnique(const w32string& sSrc,
    const w32string* psAmbigCharSet = nullptr,
    const std::vector<w32string>* pAmbigGroups = nullptr,
    w32string* psRemovedAmbigChars = nullptr);

  static w32string CreateSetOfAmbiguousChars(const w32string& aAmbigChars,
    std::vector<w32string>& ambigGroups);

  // generates a pass"word" containing characters only
  // -> where to store the password (buffer must be large enough!)
  // -> desired length
  // -> password flags (PASSW_FLAG_...)
  // <- length of the password
  int GetPassword(SecureW32String& sPassw,
    int nLength,
    int nFlags) const;

  // generates a pass"phrase" containing words and possibly characters
  // -> where to store the passphrase - buffer is resized automatically
  // -> desired number of words
  // -> characters (e.g., from a password) to combine with the words;
  //    may be an empty buffer
  // -> length of character sequence (may be 0)
  // -> passphrase flags (PASSPHR_FLAG_...)
  // -> pointer to receive number of characters related to words only
  //    (as opposed to characters from pChars)
  // <- overall length of the passphrase
  int GetPassphrase(
    SecureW32String& sDest,
    int nWords,
    const word32* pChars,
    int nCharsLen,
    int nFlags,
    int* pnNetWordsLen = nullptr) const;

  // generates a "formatted" password
  // -> destination buffer (where to store the password)
  // -> max. length of the resulting password (*without* terminating zero!)
  // -> format string; may contain "format specifiers" preceded by a '%' sign
  // -> format flags (PASSFORMAT_FLAG_...)
  // -> pointer to a previously generated password, which may be inserted into
  //    the target buffer if the format string contains '%P' (may be nullptr)
  // -> indicates *how* the password given by pszPassw was actually used
  //    if = 0: not applicable
  //    if < 0: error, see PASSFORMAT_PWUSED_...
  //    if > 0: number of bytes copied to the target buffer, may be less than
  //             the password length if the buffer is too small
  //    (may be nullptr)
  // -> indicates the first invalid specifier in the format string (may be nullptr)
  // -> estimated security of the generated password; estimation does not take
  //    into account permutations
  // <- length of the resulting password
  int GetFormatPassw(SecureW32String& sDest,
    const w32string& sFormat,
    int nFlags,
    const word32* pPassw = nullptr,
    int* pnPasswUsed = nullptr,
    w32string* pInvalidSpec = nullptr,
    double* pdSecurity = nullptr);

  // call this function to set up all character sets by providing user-defined
  // ambiguous characters and special symbols
  // -> custom character set for generating passwords (via 'GetPassword()')
  // -> re-defined ambiguous characters
  // -> re-defined special symbols
  // -> 'true': include at least one character from each subset specified in
  //    the custom character set
  // -> 'true': remove ambiguous characters from all character sets
  // -> 'true': determine subsets (such as <AZ>, <symbols>, ...) from custom
  //    character set and use these subsets for the "include at least one
  //    letter/digit/..." password options
  // <- resulting character set derived from 'sCustomChars'
  WString SetupCharSets(const WString& sCustomChars,
    const WString& sAmbigChars = "",
    const WString& sSpecialSymbols = "",
    bool blIncludeCharFromEachSubset = false,
    bool blExcludeAmbigChars = false,
    bool blDetermineSubsets = false);

  // load a word list from a file
  // -> file name (full path); if nullptr, load the default word list
  // -> maximum word length allowed in the list (does not apply to the
  //    default list)
  // <- number of words added to the list; <= 0 in case of an error
  //    if  < 0: i/o error
  //    if == 0: number of words < 2
  int LoadWordListFile(WString sFileName = "",
    int nMinWordLen = 0,
    int nMaxWordLen = 0,
    bool blConvertToLC = false);

  // returns a word from the word list (either from m_pWordList or default list)
  // -> index of the word
  // <- word
  WString GetWord(int nIndex) const;

  // create file with frequencies of phonetic trigrams (or trigraphs)
  // consisting of all possible 3-letter combinations (26^3=17,576 in total)
  // -> name of the source file; ideally, this should be a large word list or
  //    dictionary in a certain language
  // -> name of the destination file; it has the following structure:
  //    bytes 1..4      : total number (32-bit) of trigrams
  //    bytes 5..12     : entropy per character provided by the trigrams;
  //                      must be in the range 1.0 < entropy < log_2(26)
  //    bytes 13..70316 : frequencies of all possible trigrams stored as
  //                      17,576 32-bit numbers (i.e., 4 bytes each)
  // <- number of evaluated trigrams and bits of entropy per letter;
  //    min. entropy is 1.0!
  // function throws an exception in case of errors
  static std::pair<word32,double> CreateTrigramFile(const WString& sSrcFileName,
    const WString& sDestFileName);

  // load a trigram file created with CreateTrigramFile()
  // -> name of the trigram file; if empty, use default trigrams in
  //    PhoneticTrigram.h
  // <- if  < 0: i/o error
  //    if == 0: file structure is invalid
  //    if  > 0: success
  int LoadTrigramFile(WString sFileName = "");

  // generates phonetic (pronounceable) password
  // -> dest. buffer
  // -> desired length of the password
  // -> supported password flags:
  //    - PASSW_FLAG_INCLUDEUCL
  //    - PASSW_FLAG_INCLUDELCL
  //    - PASSW_FLAG_INCLUDEDIGIT
  //    - PASSW_FLAG_INCLUCDESPECIAL
  //    - PASSW_FLAG_PHONETICUPPERCASE
  //    - PASSW_FLAG_PHONETICMIXEDCASE
  // <- password length
  int GetPhoneticPassw(SecureW32String& sDest,
    int nLength,
    int nFlags) const;

  // calculate entropy for randomly permuting a "set" (i.e., collection of
  // unique elements): S = log_2(N!/(N-M!)) with N being the set size
  // and M being the number of samples taken from the permuted set
  // -> set size
  // -> number of samples from the permuted set
  // <- entropy bits
  static double CalcPermSetEntropy(int nSetSize,
    int nNumOfSamples);

  // roughly estimates the quality of a password
  // provided by the user
  // (code based on PwUtil.cpp from Dominik Reichl's KeePass)
  // -> password (null-terminated string)
  // <- security in bits
  static double EstimatePasswSecurity(const wchar_t* pwszPassw);


  // some properties...

  __property RandomGenerator* RandGen=
  { read=m_pRandGen, write=m_pRandGen };

  __property WString CustomCharSet =
  { read=GetCustomCharSetAsWString };

  __property w32string CustomCharSetW32 =
  { read=m_sCustomCharSet };

  __property CharSetType CustomCharSetType =
  { read=m_customCharSetType };

  __property double CustomCharSetEntropy =
  { read=m_dCustomCharSetEntropy };

  __property int CustomCharSetUniqueSize =
  { read=m_nCustomCharSetUniqueSize };

  __property double WordListEntropy =
  { read=m_dWordListEntropy };

  __property int WordListSize =
  { read=m_nWordListSize };

  __property double PhoneticEntropy =
  { read=m_dPhoneticEntropy };

  __property w32string WordSeparator =
  { read=m_sWordSep, write=m_sWordSep };

  __property w32string WordCharSeparator =
  { read=m_sWordCharSep, write=m_sWordCharSep };
};


#endif
