#!/usr/bin/env python

# Copyright (c) Dmitry Sapozhnikov
# Licensed under the MIT license.

""" Generates C++ header file based on the Unicode Character Database """

import os
import re
import sys
import urllib.request
import os.path
import hashlib
import datetime
import collections

UNICODESPACE = 0x110000
#ROOTURL = "https://www.unicode.org/Public/UNIDATA"
ROOTURL = "https://www.unicode.org/Public/draft/UCD/ucd/"
DATA_SOURCE = { 'GCBREAK' : (ROOTURL + '/auxiliary/GraphemeBreakProperty.txt',
                             ['CODERANGE', 'BREAK_CLASS']),
                'EAWIDTH' : (ROOTURL + '/EastAsianWidth.txt',
                             ['CODERANGE', 'EAST_ASIAN_WIDTH']),
                'UNICODE' : (ROOTURL + '/UnicodeData.txt',
                             ['CODERANGE'  , 'NAME'         , 'CATEGORY'     , 'COMBO_CLASS'  , 'BIDI_CATEGORY',
                              'DECOMP_MAP' , 'DECIMAL_VALUE', 'DIGITAL_VALUE', 'NUMERIC_VALUE', 'MIRRORED'     ,
                              'LEGACY_NAME', 'COMMENT'      , 'UPPERCASE_MAP', 'LOWERCASE_MAP', 'TITLECASE_MAP']),
                'EMOJILS' : (ROOTURL + '/emoji/emoji-data.txt',
                             ['CODEVALUE', 'EMOJI_BREAK_PROP']),
                'ALIASES' : (ROOTURL + '/NameAliases.txt',
                             ['CODEVALUE', 'ALIAS', 'TYPE']),
                "SCRIPTS" : (ROOTURL + '/Scripts.txt',
                             ['CODERANGE', 'SCRIPT_PROP']),
                "ISOCODS" : ('https://www.unicode.org/iso15924/iso15924.txt',
                             ['CODE', 'ISOCODE', 'NAME', 'FrNAME', 'PVA', 'VER', 'DATE']) }

# Value Aliases: http://www.unicode.org/Public/UNIDATA/PropertyValueAliases.txt
# classification: https://www.unicode.org/reports/tr29/#Grapheme_Cluster_Break_Property_Values
BREAKCAT = { 'Other'                 : ['any'  , 'Other'                        ],
             'CR'                    : ['cr'   , 'CR'                           ],
             'LF'                    : ['lf'   , 'LF'                           ],
             'Control'               : ['ctrl' , 'Control'                      ],
             'Extend'                : ['ext'  , 'Extend or Emoji_Modifier_Base'],
             'L'                     : ['l'    , 'HANGUL CHOSEONG'              ],
             'V'                     : ['v'    , 'HANGUL JUNGSEONG'             ],
             'T'                     : ['t'    , 'HANGUL JUNGSEONG'             ],
             'LV'                    : ['lv'   , 'HANGUL SYLLABLE'              ],
             'LVT'                   : ['lvt'  , 'HANGUL SYLLABLE'              ],
             'Regional_Indicator'    : ['ri'   , 'Regional_Indicator'           ],
             'SpacingMark'           : ['sm'   , 'SpacingMark'                  ],
             'Prepend'               : ['prep' , 'Prepend'                      ],
             'ZWJ'                   : ['zwj'  , 'ZERO WIDTH JOINER'            ],
             'Extended_Pictographic' : ['ep'   , 'Extended_Pictographic'        ],
             'EP + ZWJ'              : ['combo', 'EP + ZWJ'                     ] }

# classification: https://www.unicode.org/reports/tr44/#General_Category_Values
CATEGORY = { 'Uppercase_Letter'      : 'Lu',  # an uppercase letter
             'Lowercase_Letter'      : 'Ll',  # a lowercase letter
             'Titlecase_Letter'      : 'Lt',  # a digraphic character, with first part uppercase
             'Cased_Letter'          : 'LC',  # Lu | Ll | Lt
             'Modifier_Letter'       : 'Lm',  # a modifier letter
             'Other_Letter'          : 'Lo',  # other letters, including syllables and ideographs
             'Letter'                : 'L' ,  # Lu | Ll | Lt | Lm | Lo

             'Nonspacing_Mark'       : 'Mn',  # a nonspacing combining mark (zero advance width)
             'Spacing_Mark'          : 'Mc',  # a spacing combining mark (positive advance width)
             'Enclosing_Mark'        : 'Me',  # an enclosing combining mark
             'Mark'                  : 'M' ,  # Mn | Mc | Me

             'Decimal_Number'        : 'Nd',  # a decimal digit
             'Letter_Number'         : 'Nl',  # a letterlike numeric character
             'Other_Number'          : 'No',  # a numeric character of other type
             'Number'                : 'N' ,  # Nd | Nl | No

             'Connector_Punctuation' : 'Pc',  # a connecting punctuation mark, like a tie
             'Dash_Punctuation'      : 'Pd',  # a dash or hyphen punctuation mark
             'Open_Punctuation'      : 'Ps',  # an opening punctuation mark (of a pair)
             'Close_Punctuation'     : 'Pe',  # a closing punctuation mark (of a pair)
             'Initial_Punctuation'   : 'Pi',  # an initial quotation mark
             'Final_Punctuation'     : 'Pf',  # a final quotation mark
             'Other_Punctuation'     : 'Po',  # a punctuation mark of other type
             'Punctuation'           : 'P' ,  # Pc | Pd | Ps | Pe | Pi | Pf | Po

             'Math_Symbol'           : 'Sm',  # a symbol of mathematical use
             'Currency_Symbol'       : 'Sc',  # a currency sign
             'Modifier_Symbol'       : 'Sk',  # a non-letterlike modifier symbol
             'Other_Symbol'          : 'So',  # a symbol of other type
             'Symbol'                : 'S' ,  # Sm | Sc | Sk | So

             'Space_Separator'       : 'Zs',  # a space character (of various non-zero widths)
             'Line_Separator'        : 'Zl',  # U+2028 LINE SEPARATOR only
             'Paragraph_Separator'   : 'Zp',  # U+2029 PARAGRAPH SEPARATOR only
             'Separator'             : 'Z' ,  # Zs | Zl | Zp

             'Control'               : 'Cc',  # a C0 or C1 control code
             'Format'                : 'Cf',  # a format control character
             'Surrogate'             : 'Cs',  # a surrogate code point
             'Private_Use'           : 'Co',  # a private-use character
             'Unassigned'            : 'Cn',  # a reserved unassigned code point or a noncharacter
             'Other'                 : 'C' }  # Cc | Cf | Cs | Co | Cn

# classification: empirically
ZEROWIDTH = [CATEGORY['Control'            ], #'Cc'
             CATEGORY['Format'             ], #'Cf'
             CATEGORY['Surrogate'          ], #'Cs'
             CATEGORY['Unassigned'         ], #'Cn'
             CATEGORY['Line_Separator'     ], #'Zl'
             CATEGORY['Paragraph_Separator'], #'Zp'
             CATEGORY['Nonspacing_Mark'    ], #'Mn'
             CATEGORY['Spacing_Mark'       ], #'Mc'
             CATEGORY['Enclosing_Mark'     ]] #'Me'

# A 'Prepend' characters always have the width 'Narrow' to be the basis of the grapheme cluster.
PRINTABLE = ['Prepend', # always part of grapheme cluster
             ]

# classification https://www.unicode.org/reports/tr11/#ED6
WCWIDTHS = { 'zerowidth' : ['zero', 'non-printable' ],
             'halfwidth' : ['slim', '1x1 narrow'    ],
             'fullwidth' : ['wide', '2x1 fullwidth' ] }

CUSTOMIZE = [('0200B',        'zero', 'Nonspacing_Mark', 'Extend'), # ZWSP, ZERO WIDTH SPACE is always part of grapheme cluster and can't be the first
             ('0200C',        'zero', 'Nonspacing_Mark', 'Extend'), # ZWNJ, ZERO WIDTH NON-JOINER is always part of grapheme cluster and can't be the first
             ('D0000..D0FFF', 'zero', 'Nonspacing_Mark', 'Extend')]
#def gc(x):
#    return x * (x + 1) / 2 + 1
#for w in range(1, 5):
#    for h in range(1, 5):
#        for y in range(0, h + 1):
#            for x in range(0, w + 1):
#                tag = 'vs<{}{},{}{}>'.format(w, h, x, y)
#                code = 0xE0100 + int(gc(w) + gc(h) * 16 + x + y * 16)
#                CUSTOMIZE.append((code, tag, nspm))

# classification:  empirically
# todo except BREAKCAT = Prepend (always part of grapheme cluster)
CONTROLCP = [CATEGORY['Control'            ], #'Cc'
             CATEGORY['Format'             ], #'Cf'
             CATEGORY['Line_Separator'     ], #'Zl'
             CATEGORY['Paragraph_Separator']] #'Zp'
#NONCTRLCP = ['0x00000..0x0001F',  # C0 are processed as bytes before UTF-8 decoding (the termxs project)
#             '0x00080..0x0009F',  # C1 must be represented as C0 escaped sequences      (the termxs project)
#             '0x0200D'         ,  # ZWJ, ZERO WIDTH JOINER is always part of grapheme cluster and can't be the first
#             '0xE0000..0xE007F' ] # TAGs can't be the first grapheme cluster's codepoint: https://www.unicode.org/reports/tr51/#def_emoji_tag_sequence

NONCTRLCP = ['0200D',        # ZWJ,  ZERO WIDTH JOINER is always part of grapheme cluster and can't be the first
             'E0000..E007F'] # TAGs can't be the first grapheme cluster's codepoint: https://www.unicode.org/reports/tr51/#def_emoji_tag_sequence

#todo unify
# classification The Commands are the subset of The Controls
## CMMNDS = ['00..07','0A..1F', # C0 w/o BS, TAB
CMMNDS = ['00..1F', # C0
          '7F..9F', # C1
          '2029'  ] # Paragraph Separator

#classification: https://www.unicode.org/reports/tr11/#Recommendations
EAWIDTH = { 'NP': WCWIDTHS['zerowidth'][0],  # Non-printable
            'A' : WCWIDTHS['halfwidth'][0],  # Ambiguous
            'H' : WCWIDTHS['halfwidth'][0],  # Halfwidth
            'N' : WCWIDTHS['halfwidth'][0],  # Neutral
            'Na': WCWIDTHS['halfwidth'][0],  # Narrow
            'F' : WCWIDTHS['fullwidth'][0],  # Fullwidth
            'W' : WCWIDTHS['fullwidth'][0] } # Wide

NON_CONTROL = 'non_control'
CNTRLCLSASS = 'cntrls'
BREAKSCLASS = 'gbreak'
WCWIDTHTYPE = 'widths'
HEADER_FILE = 'unidata.hpp'
HEADER_BASE = r'''/**
 * {header} autogenerated {moment}
 *
 * Provides access to the Unicode Character Database.
 * Properties of a single Unicode character are accessed by its code point value.
 *
 * Format conventions
 *  https://www.unicode.org/reports/tr44/
 *
 * Character presentation width rules
 *  EAW:    https://www.unicode.org/reports/tr11
 *  Emoji:  https://www.unicode.org/reports/tr51
 *
 * Grapheme clusterization
 *  https://www.unicode.org/reports/tr29
 *
 * Unicode Character Database properties
 *  https://www.unicode.org/reports/tr44/#Property_Index
 *
 * All emoji characters are treated 'East Asian Wide'.
 *  Current practice is for emoji to have a square aspect ratio, deriving from their origin in Japanese.
 *  https://www.unicode.org/reports/tr51/#Display
 *
 * VS15/16
 *  Emoji presentation sequences behave as though they were East Asian Wide,
 *  regardless of their assigned East_Asian_Width property value.
 *  https://www.unicode.org/reports/tr11/#Recommendations
 *
 * Ambiguous characters
 *  Ambiguous characters behave like wide or narrow characters depending on the context.
 *  If the context cannot be established reliably, they should be treated as narrow characters by default.
 *  https://www.unicode.org/reports/tr11/#Recommendations
 *
 * Categories of the character width
 *  0 - Non-printable
 *  1 - Halfwidth
 *  2 - Fullwidth
 *
 * C0 controls 0x00..0x1F
 *  Since C0 codes are based on bytes, they are excluded from the property list of controls.
 *  They are analyzed in static tables before decoding UTF-8.
 *
 * C1 controls 0x80..0x9F
 *  The C1 codes require two bytes to be encoded in UTF-8
 *  (for instance CSI at U+009B is encoded as the bytes 0xC2, 0x9B in UTF-8),
 *  so there is no advantage to using them rather than the equivalent
 *  two-byte ESC+letter sequence, so the C1 controls are represented
 *  as C0 escaped sequences.
 *  The C1 characters appear outdated now.
 *  https://en.wikipedia.org/wiki/C0_and_C1_control_codes#Unicode
 *
 * Soft Hyphen
 *  Two ways:
 *    1. Interpret it as a command and divide the text strings into two independent.
 *    2. Append it to the last grapheme cluster.
 *
 * Printable format characters
 *  A 'Prepend' characters always have the width 'Narrow' to be the basis of the grapheme cluster.
 *  https://www.unicode.org/reports/tr29/#Table_Combining_Char_Sequences_and_Grapheme_Clusters
 *  https://www.unicode.org/reports/tr29/#GB9b
 *
 * Names and Aliases
 *  Character name aliases are immutable.
 *  https://www.unicode.org/versions/Unicode12.0.0/ch04.pdf#page=24
 *
 * Unicode Bidirectional Algorithm
 *  https://www.unicode.org/cldr/utility/bidi.jsp
 *  https://unicode.org/reports/tr9/
 *
 * Controls (should be enumerated in ascending order)
 *    command: <NON_CONTROL (paragraph break possible)
 *            C0
 *            C1
 *            \u2029 PARAGRAPH SEPARATOR
 *    visible: =NON_CONTROL - non control chars
 *     format: >NON_CONTROL
 *             All other control chars
 **/

#pragma once

#include <cstdint>
#include <vector>
#include <iterator>

namespace netxs::unidata
{{
    namespace widths
    {{
        static constexpr auto zero = 0; // 0x0 non-printable
        static constexpr auto slim = 1; // 1x1 narrow
        static constexpr auto wide = 2; // 2x1 fullwidth
    }}

    namespace gbreak
    {{
        static constexpr auto any   =  0; // Other
        static constexpr auto cr    =  1; // CR
        static constexpr auto lf    =  2; // LF
        static constexpr auto ctrl  =  3; // Control
        static constexpr auto ext   =  4; // Extend or Emoji_Modifier_Base
        static constexpr auto l     =  5; // HANGUL CHOSEONG
        static constexpr auto v     =  6; // HANGUL JUNGSEONG
        static constexpr auto t     =  7; // HANGUL JUNGSEONG
        static constexpr auto lv    =  8; // HANGUL SYLLABLE
        static constexpr auto lvt   =  9; // HANGUL SYLLABLE
        static constexpr auto ri    = 10; // Regional_Indicator
        static constexpr auto sm    = 11; // SpacingMark
        static constexpr auto prep  = 12; // Prepend
        static constexpr auto zwj   = 13; // ZERO WIDTH JOINER
        static constexpr auto ep    = 14; // Extended_Pictographic
        static constexpr auto combo = 15; // EP + ZWJ
    }}

    enum cntrls : ui32
    {{
        {cntrls}
    }};

    struct unidata;
    inline unidata const& select(ui32 cp);

    struct unidata
    {{
        ui32 ucwidth : 2; // 0 - zero, 1 - slim, 2 - wide.
        ui32 brgroup : 4;
        ui32 control : 7;
        //ui32 reserv : 19;

        constexpr unidata(unidata const&) = default;
        constexpr unidata()
            : ucwidth{{ widths::slim }},
              brgroup{{ gbreak::any }},
              control{{ cntrls::non_control }}
        {{ }}
        constexpr unidata(ui32 ucwidth, ui32 brgroup, ui32 control)
            : ucwidth{{ ucwidth }},
              brgroup{{ brgroup }},
              control{{ control }}
        {{ }}
        unidata(ui32 cp)
            : unidata{{ select(cp) }}
        {{ }}

        constexpr unidata& operator = (unidata const&) = default;

        auto is_cmd()
        {{
            return control < cntrls::non_control;
        }}

        // Unicode 15.1.0 UAX #29 https://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundary_Rules
        bool allied(unidata const& next)
        {{
            auto l = brgroup;
            auto r = next.brgroup;
            auto result = //todo use lut(l, r)
                (  l == gbreak::cr    &&  r == gbreak::lf   )  ? true: // GB3

                (  l >= gbreak::cr    &&  l <= gbreak::ctrl )  ? faux: // GB4

                (  r >= gbreak::cr    &&  r <= gbreak::ctrl )  ? faux: // GB5

                (  l == gbreak::l     && (r == gbreak::l
                                      ||  r == gbreak::v
                                      ||  r == gbreak::lv
                                      ||  r == gbreak::lvt  )) ? true: // GB6

                (( l == gbreak::lv    ||  l == gbreak::v    )
              && ( r == gbreak::v     ||  r == gbreak::t    )) ? true: // GB7

                (( l == gbreak::lvt   ||  l == gbreak::t    )
                                      &&  r == gbreak::t    )  ? true: // GB8

                (  l == gbreak::prep  ||  r == gbreak::zwj
                                      ||  r == gbreak::sm
                                      ||  r == gbreak::ext  )  ? true: // GB9,a,b

                (  l == gbreak::combo &&  r == gbreak::ep   )  ? true: // GB11

                (  l == gbreak::ri    &&  r == gbreak::ri   )  ? true: // GB12,13
                                                                 faux; // GB999
            if (l == gbreak::ep)
            {{
                brgroup = (r == gbreak::ext) ? gbreak::ep    :
                          (r == gbreak::zwj) ? gbreak::combo : r;
            }}
            else
            {{
                brgroup = (l == gbreak::ri && r == gbreak::ri) ? gbreak::any : r;
            }}
            return result;
        }}
    }};

    struct base
    {{
        static constexpr auto blocks_size = sz_t{{ {blocks_size} }};
        static constexpr auto blocks_pack = std::to_array<si32>(
        {{
            {blocks}
        }});

        static constexpr auto offset_size = sz_t{{ {offset_size} }};
        static constexpr auto offset_pack = std::to_array<si32>(
        {{
            {offset}
        }});

        static constexpr auto scripts = std::to_array<ui32>(
        {{
            {scripts}
        }});

        static constexpr auto ucspec = std::to_array<unidata>(
        {{
            {ucspec}
        }});
    }};

    template<class T, class D>
    auto unpack(D const& pack, sz_t size)
    {{
        auto data = std::vector<T>{{}};
        data.reserve(size);
        auto iter = pack.begin();
        auto tail = pack.end();
        while (iter != tail)
        {{
            auto n = *iter++;
            if (n < 0) data.insert(data.end(), -n, static_cast<T>(*iter++));
            else       data.push_back(static_cast<T>(n));
        }}
        return data;
    }}
    unidata const& select(ui32 cp)
    {{
        static auto offset = unpack<byte>(base::offset_pack, base::offset_size);
        static auto blocks = unpack<ui16>(base::blocks_pack, base::blocks_size);
        return cp > 0x10FFFF ? base::ucspec[0]
                             : base::ucspec[offset[blocks[cp >> 8] + (cp & 0xFF)]];
    }}
    auto script(ui32 cp)
    {{
        static auto data = []
        {{
            auto v = std::vector<ui16>(0x10FFFF); // ISO 15924 Script No: 0 - 999.
            auto j = 0;
            auto s = 0;
            auto l = 0;
            for (auto i = 0u; i < v.size(); i++)
            {{
                if (l == 0)
                {{
                    s = base::scripts[j] & 0xFFFF;
                    l = base::scripts[j] >> 16;
                    j++;
                }}
                v[i] = (ui16)s;
                l--;
            }}
            return v;
        }}();
        return cp > 0x10FFFF ? data[0] : data[cp];
    }}
}}
'''.strip()

def writeln(text):
    sys.stdout.write(text + '\n')
    sys.stdout.flush()
def write(text):
    sys.stdout.write(text)
    sys.stdout.flush()
def progress(code):
    if code % round(UNICODESPACE / 100) == 0:
        write('.')

def loaddata(url):
    filename = url.rsplit('/', 1)[-1]
    if not os.path.isfile(filename):
        writeln("get " + url)
        urllib.request.urlretrieve(url, filename)
    with open(filename, 'rb') as outfile:
        writeln("read " + filename)
        content = outfile.read()
    values = content.decode("utf-8").splitlines() #split('\n')
    values = [record for record in values if not record.startswith('#') and len(record) != 0]
    digest = 'SHA256#' + hashlib.sha256(content).hexdigest()
    return (values, digest)

class uniprop(object):
    def __init__(self, code):
        self.code       = code
        self.ucwidth    = EAWIDTH['N']
        self.gcbreak    = 'Other'
        self.category   = CATEGORY['Unassigned']
        self.ctrl_index = None
        self.name       = None
        self.alias      = None
        self.scriptname = 'Unknown'
        self.scriptcode = 999

    def hash(self):
        return 'ctrl{}wd{}br{}'.format(self.ctrl_index, self.ucwidth, self.gcbreak)

    def prop(self):
        return [self.ucwidth, self.gcbreak, self.ctrl_index]

class unidata(object):
    def __init__(self, src):
        url = src[0]
        fields = src[1]
        self.url = url
        self.data, self.hash = loaddata(url)
        self.fields = fields

    def props(self, *fieldlist):
        result = []
        for line in self.data:
            dataline = line.split('#', 1)
            values = [v.strip() for v in dataline[0].split(';')]
            if len(values) == len(self.fields):
                record = [values[self.fields.index(field)] for field in fieldlist]
                result.append(record)
        return result

class unirepo(object):
    def __init__(self, sources):
        self.src = { src[0] : unidata(src[1]) for src in sources.items() }

    def to_str(self):
        maxurl = str(max([len(i[1].url) for i in self.src.items()]))
        text = ''
        for item in self.src.items():
            text += ('{:<'+maxurl+'} {}\n *\t').format(item[1].url, item[1].hash)
        return text

def sequencer(scope):
    args = [int(n, 16) for n in scope.split('..')]
    return range(args[0], args[1 if len(args) > 1 else 0] + 1)
def sequencer2(start, end):
    return range(int(start, 16), int(end, 16) + 1)

def eolgenerator(length, columns, group, spc=' '):
    for i in range(1, length):
        if i % group == 0:
            yield ',\n\n            '
        elif i % columns == 0:
            yield ',\n            '
        else:
            yield ','+spc
            #yield ', '
    yield ''

def apply_category(source, chrs):
    start = ''
    for cprange, category, name in source.props('CODERANGE', 'CATEGORY', 'NAME'):
        if name.endswith(', First>'):
            start = cprange
            continue
        elif name.endswith(', Last>') and start != '':
            scope = sequencer2(start, cprange)
            start = ''
        else:
            scope = sequencer(cprange)

        for cp in scope:
            chrs[cp].category = category
            chrs[cp].name = name

def apply_gcbreaks(source, chrs):
    for cprange, boundclass in source.props('CODERANGE', 'BREAK_CLASS'):
        for cp in sequencer(cprange):
            chrs[cp].gcbreak = boundclass

def apply_eawemoji(source, chrs):
    for cprange, brprop in source.props('CODEVALUE', 'EMOJI_BREAK_PROP'):
        for cp in sequencer(cprange):
            # why the emoji should be always wide?
            #chrs[cp].ucwidth = EAWIDTH['W']
            if (brprop == 'Extended_Pictographic'): # https://www.unicode.org/reports/tr29/#GB11
                chrs[cp].gcbreak = brprop
            elif (brprop == 'Emoji_Modifier_Base'): # https://www.unicode.org/reports/tr29/#Extend
                chrs[cp].gcbreak = 'Extend'

def apply_eawidths(source, chrs):
    for cprange, eawidth in source.props('CODERANGE', 'EAST_ASIAN_WIDTH'):
        ucwidth = EAWIDTH[eawidth]
        for cp in sequencer(cprange):
            chrs[cp].ucwidth = ucwidth

def apply_wscripts(isocodes_src, scripts_src, chrs):
    isocodes = {}
    for isocode, name, pva in isocodes_src.props('ISOCODE', 'NAME', 'PVA'):
        if pva: isocodes[pva] = int(isocode)
        else :  isocodes[name.replace(" ", "_").replace("-", "_")] = int(isocode)

    for cprange, scriptname in scripts_src.props('CODERANGE', 'SCRIPT_PROP'):
        for cp in sequencer(cprange):
            chrs[cp].scriptname = scriptname
            chrs[cp].scriptcode = isocodes[scriptname]

def apply_acronyms(source, chrs):
    for (cpval, alias, cptype) in source.props('CODEVALUE', 'ALIAS', 'TYPE'):
        cp = int(cpval, 16)
        if cptype == 'abbreviation': # https://www.unicode.org/versions/Unicode12.0.0/ch04.pdf#page=24
            chrs[cp].alias = alias
        elif chrs[cp].name.startswith('<'):
            chrs[cp].name = alias

def apply_nonprint(categories, printable, chrs):
    for cp in chrs:
        if cp.category in categories and cp.gcbreak not in printable:
            cp.ucwidth = EAWIDTH['NP']

def apply_customcp(recs, chrs):
    for cprange, width, category, breakclass in recs:
        for cp in sequencer(cprange):
            chrs[cp].ucwidth  = width
            chrs[cp].category = category
            chrs[cp].gcbreak  = breakclass

#def apply_commands(commands, excluded, printable, chrs):
#    index = 0
#    noncmds = set()
#    for cs in excluded:
#        noncmds |= set(list(sequencer(cs)))
#    for cp in chrs:
#        # TODO sort CMMNDS
#        #    c0  0..1F
#        #    c1  7F..9F
#        #    PS  2029
#        #   -NON_CONTROL
#        #    other controls
#
#        if cp.category in commands:
#            if cp.code not in noncmds and cp.gcbreak not in printable:
#                cp.ctrl_index = index
#                index += 1
def apply_commands(commands, excluded, printable, chrs):
    noncmds = set()
    for cs in excluded:
        noncmds.update(sequencer(cs))
        #noncmds |= set(list(sequencer(cs)))

    cntrls = [cp for cp in chrs if (cp.category in commands) and (cp.code not in noncmds and cp.gcbreak not in printable)]
    cmmnds = []
    formts = []

    CMMNDS_SET = set()
    for cs in CMMNDS:
        CMMNDS_SET.update(sequencer(cs))

    for cp in cntrls:
        #todo optimize
        if cp.code in CMMNDS_SET:
            cmmnds.append(cp)
        else:
            formts.append(cp)

    index = 0
    for cp in cmmnds:
        cp.ctrl_index = index
        index += 1

    noncmd_id = index
    index += 1

    for cp in formts:
        cp.ctrl_index = index
        index += 1

    writeln('ctrl characters: %d' % index)
    return noncmd_id

def get_name(text):
    return text.replace(' ', '_').replace('-', '_')

def sortFirst(val):
    return val[1]

chrs = [uniprop(cp) for cp in range(UNICODESPACE)]
data = unirepo(DATA_SOURCE)

apply_category(data.src['UNICODE'], chrs)
apply_eawidths(data.src['EAWIDTH'], chrs)
apply_gcbreaks(data.src['GCBREAK'], chrs)
apply_eawemoji(data.src['EMOJILS'], chrs)
apply_acronyms(data.src['ALIASES'], chrs)
apply_customcp(CUSTOMIZE,           chrs)
apply_nonprint(set(ZEROWIDTH), set(PRINTABLE), chrs)
noncmd_id = apply_commands(set(CONTROLCP), NONCTRLCP, set(PRINTABLE), chrs)
apply_wscripts(data.src['ISOCODS'], data.src['SCRIPTS'], chrs)

# 00 33 7 555 00
# ranges: 0,2 3,2 7,1 5,3 0,2
script_ranges = []
charscount = 0
scriptcode = chrs[0].scriptcode
maxlen = 0
for cp in chrs:
    if cp.scriptcode != scriptcode or charscount == 65535:
        script_ranges.append(scriptcode + (charscount << 16))
        scriptcode = cp.scriptcode
        if charscount > maxlen:
            maxlen = charscount
        charscount = 0
    charscount += 1
script_ranges.extend([scriptcode, charscount])

scripts = ''
for i, value in enumerate(script_ranges):
    scripts += '            ' if i and i % 20 == 0 else ''
    scripts += '0x%X,' % value
    scripts += '\n' if (i + 1) % 20 == 0 else ' '

#control_list = { 0 : (0, 'NON FORMAT CHARACTER', 'NON_FORMAT', 0) }
#control_list.update({ cp.code: (cp.ctrl_index, cp.name, cp.alias, cp.code) for cp in chrs if not cp.ctrl_index is None })
#control_list = { cp.code: (cp.ctrl_index, cp.name, cp.alias, cp.code) for cp in chrs if not cp.ctrl_index is None }

control_list = { -1 : (noncmd_id, 'NON CONTROL', 'non_control', -1) }
control_list.update({ cp.code: (cp.ctrl_index, cp.name, cp.alias, cp.code) for cp in chrs if not cp.ctrl_index is None })

cntrls = ''
control_idx = []
#for i, (cpval, (cpctrlidx, cpname, cpalias, cpcode)) in enumerate(control_list.items()):
#for i, (cpval, (cpctrlidx, cpname, cpalias, cpcode)) in enumerate(list(control_list.items()).sort(key = sortFirst)):
a = control_list.items()
b = list(a)
b.sort(key = sortFirst)
mass = enumerate(b)
for i, (cpval, (cpctrlidx, cpname, cpalias, cpcode)) in mass:
    alias = cpalias if cpalias else get_name(cpname)
    control_idx.append(alias)
    cntrls += '        ' if i != 0 else ''
    hexcode = '0x%X' % cpcode if cpcode != -1 else cpcode
    cntrls += '{:<42}, // {:>3} {:<5} {}\n'.format(alias.lower(), i, hexcode, cpname)
    #cntrls += '\n' if i != len(control_list) - 1 else ''
cntrls += '        {:<42}, // {:>3}'.format('count', len(b))

base = uniprop(0)
ucspec_index = collections.OrderedDict([(base.hash(), base.prop())])
for cp in chrs:
    index = cp.hash()
    if not index in ucspec_index:
        ucspec_index[index] = cp.prop()

ucspec = ''
for i, (key, (wide, brgroup, ctrl_id)) in enumerate(ucspec_index.items()):
    ctrlname = control_idx[ctrl_id] if ctrl_id else control_idx[noncmd_id]
    ucspec += '            ' if i != 0 else ''
    ucspec += '{{ {}, {:<12}, {:<30} }},  // {:>3}'.format(WCWIDTHTYPE + '::%s' % wide,
                                                           BREAKSCLASS + '::%s' % BREAKCAT[brgroup][0],
                                                           CNTRLCLSASS + '::%s' % ctrlname.lower(),
                                                           i)
    ucspec += '\n' if i != len(ucspec_index) - 1 else ''

#breaks = ''
#for i, (key, (value, comment)) in enumerate(BREAKCAT.items()):
#    breaks += '        ' if i != 0 else ''
#    breaks += '{:<8},  // {}'.format(value, comment)
#    breaks += '\n' if i != len(BREAKCAT) - 1 else ''

#widths = ''
#for i, (key, (value, comment)) in enumerate(WCWIDTHS.items()):
#    widths += '            ' if i != 0 else ''
#    widths += '{:<4},  // {}'.format(value, comment)
#    widths += '\n' if i != len(WCWIDTHS) - 1 else ''

offset_index = []
blocks_index = []
block_size = 0x100

write('packing')

for code in range(UNICODESPACE):
    if code % block_size == 0:
        blocks_temp = []
        for cp in range(code, code + block_size):
            index = chrs[cp].hash()
            blocks_temp.append(list(ucspec_index.keys()).index(index))
        if blocks_temp in offset_index:
            old_index = offset_index.index(blocks_temp)
            blocks_index.append(old_index * block_size)
        else:
            blocks_index.append(len(offset_index) * block_size)
            offset_index.append(blocks_temp)
    progress(code)

writeln('100%')

offset = ''
eol = eolgenerator(len(offset_index * block_size), 16, block_size, '')
cur_prop = -1
size = 0
offset_size = 0
for pos in offset_index:
    for prop in pos:
        offset_size += 1
        if prop != cur_prop and size != 0:
            if size != 1:
                offset += '{:>3}'.format(str(-size)) + next(eol)
            offset += '{:>3}'.format(str(cur_prop)) + next(eol)
            size = 0
        if size == 0:
            cur_prop = prop
        size += 1
if size != 1:
    offset += '{:>3}'.format(str(-size)) + next(eol)
offset += '{:>3}'.format(str(cur_prop))

#eol = eolgenerator(len(offset_index * block_size), 16, block_size, '')
#for pos in offset_index:
#    for prop in pos:
#        offset += '{:>3}'.format(str(prop)) + next(eol)

blocks = ''
eol = eolgenerator(len(blocks_index), 10, 0xffff)
cur_prop = -1
size = 0
blocks_size = 0
for block in blocks_index:
    blocks_size += 1
    #blocks += str(block) + next(eol)
    if block != cur_prop and size != 0:
        if size != 1:
            blocks += '{:>3}'.format(str(-size)) + next(eol)
        blocks += '{:>3}'.format(str(cur_prop)) + next(eol)
        size = 0
    if size == 0:
        cur_prop = block
    size += 1
if size != 1:
    blocks += '{:>3}'.format(str(-size)) + next(eol)
blocks += '{:>3}'.format(str(cur_prop))


#eol = eolgenerator(len(blocks_index), 10, 0xffff)
#for block in blocks_index:
#    blocks += str(block) + next(eol)

#breaks_impl = { 'break_type' : BREAKSCLASS + '::type' }
#breaks_impl.update({ 'break_%s' % var : BREAKSCLASS + '::%s' % var for i, (key, (var, comment)) in enumerate(BREAKCAT.items()) })
#allied =  ALLIED_IMPL.format(**breaks_impl)

fields = {'header': HEADER_FILE,
          'moment': datetime.datetime.today(),
          'cntrls': cntrls,
          'blocks': blocks,
          'blocks_size': blocks_size,
          'offset': offset,
          'offset_size': offset_size,
          'scripts': scripts,
          'ucspec': ucspec }

writeln('spec count: %s' % len(ucspec_index))

header_content = HEADER_BASE.format(**fields)

with open(HEADER_FILE, 'w', newline = '\n') as f: f.write(header_content)
writeln("done " + HEADER_FILE)
