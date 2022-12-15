// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_TEXT_HPP
#define NETXS_APP_TEXT_HPP

namespace netxs::events::userland
{
    struct textancy
    {
        EVENTPACK(textancy, netxs::events::userland::root::custom )
        {
            GROUP_XS( ui, input::hids ),

            SUBSET_XS( ui )
            {
                EVENT_XS( create  , input::hids ),
                GROUP_XS( split   , input::hids ),

                SUBSET_XS( split )
                {
                    EVENT_XS( hz, input::hids ),
                };
            };
        };
    };
}

// text: Text editor.
namespace netxs::app::textancy
{
    using events = netxs::events::userland::textancy;

    namespace
    {
        text topic3 = R"( 
Plain text vs. rich text

There are important differences between plain text (created and edited by 
text editors) and rich text (such as that created by word processors or 
desktop publishing software).

Plain text exclusively consists of character representation. Each character 
is represented by a fixed-length sequence of one, two, or four bytes, or as a 
variable-length sequence of one to four bytes, in accordance to specific 
character encoding conventions, such as ASCII, ISO/IEC 2022, UTF-8, or 
Unicode. These conventions define many printable characters, but also 
non-printing characters that control the flow of the text, such as space, 
line break, and page break. Plain text contains no other information about 
the text itself, not even the character encoding convention employed. Plain 
text is stored in text files, although text files do not exclusively store 
plain text. In the early days of computers, plain text was displayed using a 
monospace font, such that horizontal alignment and columnar formatting were 
sometimes done using whitespace characters. For compatibility reasons, this 
tradition has not changed.

Rich text, on the other hand, may contain metadata, character formatting data 
(e.g. typeface, size, weight and style), paragraph formatting data (e.g. 
indentation, alignment, letter and word distribution, and space between lines 
or other paragraphs), and page specification data (e.g. size, margin and 
reading direction). Rich text can be very complex. Rich text can be saved in 
binary format (e.g. DOC), text files adhering to a markup language (e.g. RTF 
or HTML), or in a hybrid form of both (e.g. Office Open XML).

Text editors are intended to open and save text files containing either plain 
text or anything that can be interpreted as plain text, including the markup 
for rich text or the markup for something else (e.g. SVG).

History

Before text editors existed, computer text was punched into cards with 
keypunch machines. Physical boxes of these thin cardboard cards were then 
inserted into a card-reader. Magnetic tape and disk "card-image" files 
created from such card decks often had no line-separation characters at all, 
and assumed fixed-length 80-character records. An alternative to cards was 
punched paper tape. It could be created by some teleprinters (such as the 
Teletype), which used special characters to indicate ends of records.

The first text editors were "line editors" oriented to teleprinter- or 
typewriter-style terminals without displays. Commands (often a single 
keystroke) effected edits to a file at an imaginary insertion point called 
the "cursor". Edits were verified by typing a command to print a small 
section of the file, and periodically by printing the entire file. In some 
line editors, the cursor could be moved by commands that specified the line 
number in the file, text strings (context) for which to search, and 
eventually regular expressions. Line editors were major improvements over 
keypunching. Some line editors could be used by keypunch; editing commands 
could be taken from a deck of cards and applied to a specified file. Some 
common line editors supported a "verify" mode in which change commands 
displayed the altered lines.

When computer terminals with video screens became available, screen-based 
text editors (sometimes called just "screen editors") became common. One of 
the earliest full-screen editors was O26, which was written for the operator 
console of the CDC 6000 series computers in 1967. Another early full-screen 
editor was vi. Written in the 1970s, it is still a standard editor on Unix 
and Linux operating systems. Also written in the 1970s was the UCSD Pascal 
Screen Oriented Editor, which was optimized both for indented source code as 
well as general text. Emacs, one of the first free and open source software 
projects, is another early full-screen or real-time editor, one that was 
ported to many systems. A full-screen editor's ease-of-use and speed 
(compared to the line-based editors) motivated many early purchases of video 
terminals.

The core data structure in a text editor is the one that manages the string 
(sequence of characters) or list of records that represents the current state 
of the file being edited. While the former could be stored in a single long 
consecutive array of characters, the desire for text editors that could more 
quickly insert text, delete text, and undo/redo previous edits led to the 
development of more complicated sequence data structures. A typical text 
editor uses a gap buffer, a linked list of lines (as in PaperClip), a piece 
table, or a rope, as its sequence data structure.

Types of text editors

Some text editors are small and simple, while others offer broad and complex 
functions. For example, Unix and Unix-like operating systems have the pico 
editor (or a variant), but many also include the vi and Emacs editors. 
Microsoft Windows systems come with the simple Notepad, though many 
people—especially programmers—prefer other editors with more features. Under 
Apple Macintosh's classic Mac OS there was the native SimpleText, which was 
replaced in Mac OS X by TextEdit, which combines features of a text editor 
with those typical of a word processor such as rulers, margins and multiple 
font selection. These features are not available simultaneously, but must be 
switched by user command, or through the program automatically determining 
the file type.

Most word processors can read and write files in plain text format, allowing 
them to open files saved from text editors. Saving these files from a word 
processor, however, requires ensuring the file is written in plain text 
format, and that any text encoding or BOM settings won't obscure the file for 
its intended use. Non-WYSIWYG word processors, such as WordStar, are more 
easily pressed into service as text editors, and in fact were commonly used 
as such during the 1980s. The default file format of these word processors 
often resembles a markup language, with the basic format being plain text and 
visual formatting achieved using non-printing control characters or escape 
sequences. Later word processors like Microsoft Word store their files in a 
binary format and are almost never used to edit plain text files.

Some text editors can edit unusually large files such as log files or an 
entire database placed in a single file. Simpler text editors may just read 
files into the computer's main memory. With larger files, this may be a slow 
process, and the entire file may not fit. Some text editors do not let the 
user start editing until this read-in is complete. Editing performance also 
often suffers in nonspecialized editors, with the editor taking seconds or 
even minutes to respond to keystrokes or navigation commands. Specialized 
editors have optimizations such as only storing the visible portion of large 
files in memory, improving editing performance.

Some editors are programmable, meaning, e.g., they can be customized for 
specific uses. With a programmable editor it is easy to automate repetitive 
tasks or, add new functionality or even implement a new application within 
the framework of the editor. One common motive for customizing is to make a 
text editor use the commands of another text editor with which the user is 
more familiar, or to duplicate missing functionality the user has come to 
depend on. Software developers often use editor customizations tailored to 
the programming language or development environment they are working in. The 
programmability of some text editors is limited to enhancing the core editing 
functionality of the program, but Emacs can be extended far beyond editing 
text files—for web browsing, reading email, online chat, managing files or 
playing games and is often thought of as a Lisp execution environment with a 
Text User Interface. Emacs can even be programmed to emulate Vi, its rival in 
the traditional editor wars of Unix culture.

An important group of programmable editors uses REXX as a scripting language. 
These "orthodox editors" contain a "command line" into which commands and 
macros can be typed and text lines into which line commands and macros can be 
typed. Most such editors are derivatives of ISPF/PDF EDIT or of XEDIT, IBM's 
flagship editor for VM/SP through z/VM. Among them are THE, KEDIT, X2, 
Uni-edit, and SEDIT.

A text editor written or customized for a specific use can determine what the 
user is editing and assist the user, often by completing programming terms 
and showing tooltips with relevant documentation. Many text editors for 
software developers include source code syntax highlighting and automatic 
indentation to make programs easier to read and write. Programming editors 
often let the user select the name of an include file, function or variable, 
then jump to its definition. Some also allow for easy navigation back to the 
original section of code by storing the initial cursor location or by 
displaying the requested definition in a popup window or temporary buffer. 
Some editors implement this ability themselves, but often an auxiliary 
utility like ctags is used to locate the definitions.

)";

        auto build = [](text cwd, text arg, xml::settings& config, text patch)
        {
            auto highlight_color = skin::color(tone::highlight);

            auto window = ui::cake::ctor();
            window->plugin<pro::focus>()
                  ->plugin<pro::track>()
                  ->plugin<pro::acryl>()
                  ->plugin<pro::cache>()
                  ->invoke([&](auto& boss)
                  {
                      boss.keybd.accept(true);
                      boss.SUBMIT(tier::anycast, e2::form::quit, item)
                      {
                          boss.base::template riseup<tier::release>(e2::form::quit, item);
                      };
                      boss.SUBMIT(tier::release, e2::form::upon::vtree::attached, parent)
                      {
                          static auto i = 0; i++;
                          auto title = ansi::jet(bias::center).add("Text Editor\n ~/Untitled ", i, ".txt");
                          boss.base::template riseup<tier::preview>(e2::form::prop::ui::header, title);
                      };
                  });
            auto object = window->attach(ui::fork::ctor(axis::Y))
                                ->colors(whitelt, 0xA05f1a00);
                auto menu = object->attach(slot::_1, app::shared::main_menu(config));
                auto body_area = object->attach(slot::_2, ui::fork::ctor(axis::Y));
                    auto fields = body_area->attach(slot::_1, ui::pads::ctor(dent{ 1,1 }));
                        auto layers = fields->attach(ui::cake::ctor());
                            auto scroll = layers->attach(ui::rail::ctor())
                                                ->plugin<pro::limit>(twod{ 4,3 }, twod{ -1,-1 });
                                auto edit_box = scroll->attach(ui::post::ctor(true))
                                                      ->plugin<pro::caret>(true, faux, twod{ 25,1 })
                                                      ->colors(blackdk, whitelt)
                                                      ->upload(ansi::wrp(wrap::off).mgl(1)
                                                      .add(topic3)
                                                      .fgc(highlight_color.bgc())
                                                      .add("From Wikipedia, the free encyclopedia"));
                    auto status_line = body_area->attach(slot::_2, ui::post::ctor())
                                                ->plugin<pro::limit>(twod{ 1,1 }, twod{ -1,1 })
                                                ->upload(ansi::wrp(wrap::off).mgl(1).mgr(1).jet(bias::right).fgc(whitedk)
                                                    .add("INS  Sel: 0:0  Col: 26  Ln: 2/148").nil());
                        layers->attach(app::shared::scroll_bars(scroll));
            return window;
        };
    }

    app::shared::initialize builder{ "text", build };
}

#endif // NETXS_APP_TEXT_HPP